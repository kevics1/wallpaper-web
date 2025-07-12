# -*- coding: utf-8 -*-
"""
庐山地图处理核心模块
"""

import os
import tempfile
from qgis.core import (
    QgsProject, QgsVectorLayer, QgsRasterLayer, QgsMapLayer,
    QgsCoordinateReferenceSystem, QgsCoordinateTransform,
    QgsProcessingFeedback, QgsProcessingContext, QgsProcessingUtils,
    QgsGeometry, QgsFeature, QgsVectorFileWriter, QgsFields, QgsField,
    QgsWkbTypes, QgsSymbol, QgsRendererCategory, QgsCategorizedSymbolRenderer,
    QgsSimpleMarkerSymbolLayer, QgsSimpleLineSymbolLayer, QgsSimpleFillSymbolLayer,
    QgsMarkerSymbol, QgsLineSymbol, QgsFillSymbol, QgsSvgMarkerSymbolLayer,
    QgsVectorLayerSimpleLabeling, QgsPalLayerSettings, QgsTextFormat,
    QgsTextBufferSettings, QgsContourSettings, QgsRasterContourRenderer,
    QgsColorRampShader, QgsRasterShader, QgsSingleBandPseudoColorRenderer,
    QgsHillshadeRenderer, QgsMapSettings, QgsMapRendererSequentialJob,
    QgsRectangle, QgsPointXY, QgsMessageLog, Qgis
)
from qgis.PyQt.QtCore import QVariant, QThread, pyqtSignal, QObject
from qgis.PyQt.QtGui import QColor, QFont
from qgis.analysis import QgsNativeAlgorithms
import processing
import requests
import zipfile
import io


class LushanMapProcessor(QObject):
    """庐山地图处理器"""
    
    # 信号定义
    statusChanged = pyqtSignal(str, str)  # 状态类型，状态消息
    progressChanged = pyqtSignal(int)  # 进度百分比
    finished = pyqtSignal(bool, str)  # 是否成功，消息
    
    def __init__(self, iface):
        super().__init__()
        self.iface = iface
        self.project = QgsProject.instance()
        self.target_crs = QgsCoordinateReferenceSystem("EPSG:32650")
        self.mapframe_layer = None
        self.output_path = tempfile.gettempdir()
        
    def set_output_path(self, path):
        """设置输出路径"""
        self.output_path = path
        if not os.path.exists(self.output_path):
            os.makedirs(self.output_path)
        
    def set_target_crs(self, crs_string):
        """设置目标坐标系"""
        try:
            self.target_crs = QgsCoordinateReferenceSystem(crs_string)
            if not self.target_crs.isValid():
                raise ValueError(f"无效的坐标系: {crs_string}")
        except Exception as e:
            self.statusChanged.emit("error", f"设置坐标系错误: {str(e)}")
    
    def create_mapframe_layer(self):
        """创建图廓层"""
        try:
            # 创建一个临时矢量图层
            layer = QgsVectorLayer(
                f"Polygon?crs={self.target_crs.authid()}", 
                "图廓层", 
                "memory"
            )
            
            if not layer.isValid():
                raise Exception("无法创建图廓层")
            
            # 添加字段
            provider = layer.dataProvider()
            provider.addAttributes([
                QgsField("name", QVariant.String),
                QgsField("area", QVariant.Double)
            ])
            layer.updateFields()
            
            # 添加到项目
            self.project.addMapLayer(layer)
            self.mapframe_layer = layer
            
            # 设置样式
            self._style_mapframe_layer(layer)
            
            self.statusChanged.emit("mapframe", "已创建，请手动绘制图廓范围")
            return True
            
        except Exception as e:
            self.statusChanged.emit("mapframe", f"创建失败：{str(e)}")
            return False
            
    def _style_mapframe_layer(self, layer):
        """设置图廓层样式"""
        try:
            # 创建简单填充符号
            symbol = QgsFillSymbol.createSimple({
                'color': 'transparent',
                'color_border': 'red',
                'width_border': '0.5',
                'style_border': 'dash'
            })
            
            # 应用符号
            layer.renderer().setSymbol(symbol)
            layer.triggerRepaint()
            
        except Exception as e:
            QgsMessageLog.logMessage(f"设置图廓层样式失败：{str(e)}", "LushanSuite", Qgis.Warning)
            
    def process_dem(self, dem_path):
        """处理DEM数据"""
        try:
            self.statusChanged.emit("dem", "开始处理DEM...")
            
            # 加载DEM
            dem_layer = QgsRasterLayer(dem_path, "原始DEM")
            if not dem_layer.isValid():
                raise Exception("无法加载DEM文件")
            
            # 检查图廓层
            if not self.mapframe_layer or self.mapframe_layer.featureCount() == 0:
                raise Exception("请先创建并绘制图廓层")
            
            # 获取图廓范围
            mapframe_extent = self.mapframe_layer.extent()
            
            # 1. 裁剪DEM
            self.statusChanged.emit("dem", "裁剪DEM...")
            clipped_dem_path = os.path.join(self.output_path, "clipped_dem.tif")
            
            processing.run("gdal:cliprasterbyextent", {
                'INPUT': dem_path,
                'PROJWIN': f"{mapframe_extent.xMinimum()},{mapframe_extent.xMaximum()},{mapframe_extent.yMinimum()},{mapframe_extent.yMaximum()}",
                'OUTPUT': clipped_dem_path
            })
            
            # 2. 投影转换
            self.statusChanged.emit("dem", "投影转换...")
            projected_dem_path = os.path.join(self.output_path, "dem.tif")
            
            processing.run("gdal:warpreproject", {
                'INPUT': clipped_dem_path,
                'TARGET_CRS': self.target_crs,
                'OUTPUT': projected_dem_path
            })
            
            # 3. 创建山体阴影
            self.statusChanged.emit("dem", "创建山体阴影...")
            shade_path = os.path.join(self.output_path, "shade.tif")
            
            processing.run("gdal:hillshade", {
                'INPUT': projected_dem_path,
                'Z_FACTOR': 1.0,
                'AZIMUTH': 315,
                'ALTITUDE': 35,
                'OUTPUT': shade_path
            })
            
            # 4. 加载处理后的图层
            self._load_processed_dem_layers(projected_dem_path, shade_path)
            
            # 5. 生成等高线
            self._generate_contours(projected_dem_path)
            
            self.statusChanged.emit("dem", "DEM处理完成")
            return True
            
        except Exception as e:
            self.statusChanged.emit("dem", f"处理失败：{str(e)}")
            return False
            
    def _load_processed_dem_layers(self, dem_path, shade_path):
        """加载处理后的DEM图层"""
        try:
            # 加载山体阴影
            shade_layer = QgsRasterLayer(shade_path, "山体阴影")
            if shade_layer.isValid():
                # 设置渲染器
                renderer = QgsHillshadeRenderer(shade_layer.dataProvider(), 1, 315, 35)
                shade_layer.setRenderer(renderer)
                self.project.addMapLayer(shade_layer)
                
            # 加载DEM并设置样式
            dem_layer = QgsRasterLayer(dem_path, "DEM")
            if dem_layer.isValid():
                self.project.addMapLayer(dem_layer)
                self._style_dem_layer(dem_layer)
                
                # 设置透明度
                dem_layer.renderer().setOpacity(0.8)
                
        except Exception as e:
            QgsMessageLog.logMessage(f"加载DEM图层失败：{str(e)}", "LushanSuite", Qgis.Warning)
            
    def _style_dem_layer(self, layer):
        """设置DEM图层样式"""
        try:
            # 创建颜色坡度渲染器
            shader = QgsRasterShader()
            color_ramp_shader = QgsColorRampShader()
            
            # 获取统计信息
            stats = layer.dataProvider().bandStatistics(1)
            min_val = stats.minimumValue
            max_val = stats.maximumValue
            
            # 设置颜色节点
            color_ramp_items = []
            color_ramp_items.append(QgsColorRampShader.ColorRampItem(
                min_val, QColor("#d8e5be"), "0%"
            ))
            color_ramp_items.append(QgsColorRampShader.ColorRampItem(
                min_val + (max_val - min_val) * 0.29, QColor("#add59c"), "29%"
            ))
            color_ramp_items.append(QgsColorRampShader.ColorRampItem(
                min_val + (max_val - min_val) * 0.89, QColor("#4caf50"), "89%"
            ))
            color_ramp_items.append(QgsColorRampShader.ColorRampItem(
                max_val, QColor("#006603"), "100%"
            ))
            
            color_ramp_shader.setColorRampItemList(color_ramp_items)
            color_ramp_shader.setColorRampType(QgsColorRampShader.Interpolated)
            
            shader.setRasterShaderFunction(color_ramp_shader)
            
            # 创建渲染器
            renderer = QgsSingleBandPseudoColorRenderer(layer.dataProvider(), 1, shader)
            layer.setRenderer(renderer)
            
            layer.triggerRepaint()
            
        except Exception as e:
            QgsMessageLog.logMessage(f"设置DEM样式失败：{str(e)}", "LushanSuite", Qgis.Warning)
            
    def _generate_contours(self, dem_path):
        """生成等高线"""
        try:
            # 100m等高线
            contour_100_path = os.path.join(self.output_path, "contour_100m.shp")
            processing.run("gdal:contour", {
                'INPUT': dem_path,
                'INTERVAL': 100,
                'OUTPUT': contour_100_path
            })
            
            # 20m等高线
            contour_20_path = os.path.join(self.output_path, "contour_20m.shp")
            processing.run("gdal:contour", {
                'INPUT': dem_path,
                'INTERVAL': 20,
                'OUTPUT': contour_20_path
            })
            
            # 加载等高线图层
            contour_100_layer = QgsVectorLayer(contour_100_path, "等高线_100m", "ogr")
            contour_20_layer = QgsVectorLayer(contour_20_path, "等高线_20m", "ogr")
            
            if contour_100_layer.isValid():
                self.project.addMapLayer(contour_100_layer)
                self._style_contour_layer(contour_100_layer, True)
                
            if contour_20_layer.isValid():
                self.project.addMapLayer(contour_20_layer)
                self._style_contour_layer(contour_20_layer, False)
                
        except Exception as e:
            QgsMessageLog.logMessage(f"生成等高线失败：{str(e)}", "LushanSuite", Qgis.Warning)
            
    def _style_contour_layer(self, layer, is_major):
        """设置等高线样式"""
        try:
            if is_major:  # 100m等高线
                symbol = QgsLineSymbol.createSimple({
                    'color': '#fffac1',
                    'width': '0.3',
                    'width_unit': 'MM'
                })
                layer.renderer().setSymbol(symbol)
                layer.setOpacity(0.6)
                
                # 设置标注
                self._set_contour_labeling(layer)
                
            else:  # 20m等高线
                symbol = QgsLineSymbol.createSimple({
                    'color': '#fff5a5',
                    'width': '0.1',
                    'width_unit': 'MM'
                })
                layer.renderer().setSymbol(symbol)
                layer.setOpacity(0.5)
                
            layer.triggerRepaint()
            
        except Exception as e:
            QgsMessageLog.logMessage(f"设置等高线样式失败：{str(e)}", "LushanSuite", Qgis.Warning)
            
    def _set_contour_labeling(self, layer):
        """设置等高线标注"""
        try:
            # 创建标注设置
            pal_settings = QgsPalLayerSettings()
            pal_settings.fieldName = "ELEV"
            pal_settings.enabled = True
            
            # 文本格式
            text_format = QgsTextFormat()
            text_format.setFont(QFont("SimSun", 6))
            text_format.setSize(6)
            text_format.setColor(QColor("black"))
            
            pal_settings.setFormat(text_format)
            
            # 位置设置
            pal_settings.placement = QgsPalLayerSettings.Line
            pal_settings.placementFlags = QgsPalLayerSettings.OnLine
            
            # 应用标注
            labeling = QgsVectorLayerSimpleLabeling(pal_settings)
            layer.setLabeling(labeling)
            layer.setLabelsEnabled(True)
            
        except Exception as e:
            QgsMessageLog.logMessage(f"设置等高线标注失败：{str(e)}", "LushanSuite", Qgis.Warning)
            
    def download_osm_data(self):
        """下载OSM数据"""
        try:
            self.statusChanged.emit("download", "开始下载OSM数据...")
            
            if not self.mapframe_layer or self.mapframe_layer.featureCount() == 0:
                raise Exception("请先创建并绘制图廓层")
            
            # 获取图廓范围
            extent = self.mapframe_layer.extent()
            
            # 定义下载配置
            download_configs = [
                {
                    'key': 'highway',
                    'values': ['bus_stop', 'bus_station', 'railway_station'],
                    'layer_name': '交通站点&索道',
                    'geometry_type': 'points'
                },
                {
                    'key': 'natural',
                    'values': ['peak'],
                    'layer_name': '山峰',
                    'geometry_type': 'points'
                },
                {
                    'key': 'highway',
                    'values': ['footway', 'path', 'pedestrian', 'primary', 'residential', 
                              'secondary', 'service', 'steps', 'tertiary', 'track', 'unclassified'],
                    'layer_name': '道路',
                    'geometry_type': 'lines'
                },
                {
                    'key': 'waterway',
                    'values': ['stream'],
                    'layer_name': '河流',
                    'geometry_type': 'lines'
                },
                {
                    'key': 'natural',
                    'values': ['water', 'reservoir'],
                    'layer_name': '湖泊',
                    'geometry_type': 'multipolygons'
                },
                {
                    'key': 'landuse',
                    'values': ['residential'],
                    'layer_name': '土地利用范围',
                    'geometry_type': 'multipolygons'
                }
            ]
            
            # 逐个下载数据
            for i, config in enumerate(download_configs):
                self.statusChanged.emit("download", f"正在下载 {config['layer_name']}...")
                self._download_single_osm_layer(extent, config)
                self.progressChanged.emit(int((i+1)/len(download_configs)*100))
                
            self.statusChanged.emit("download", "OSM数据下载完成")
            return True
            
        except Exception as e:
            self.statusChanged.emit("download", f"下载失败：{str(e)}")
            return False
            
    def _download_single_osm_layer(self, extent, config):
        """下载单个OSM图层"""
        try:
            # 构建查询字符串
            query_parts = []
            for value in config['values']:
                query_parts.append(f'"{config["key"]}"="{value}"')
            query = ' OR '.join(query_parts)
            
            # 构建overpass查询
            bbox = f"{extent.yMinimum()},{extent.xMinimum()},{extent.yMaximum()},{extent.xMaximum()}"
            overpass_query = f"""
            [out:xml][timeout:180];
            (
                node[{query}]({bbox});
                way[{query}]({bbox});
                relation[{query}]({bbox});
            );
            out body;
            >;
            out skel qt;
            """
            
            # 发送请求到Overpass API
            url = "https://overpass-api.de/api/interpreter"
            response = requests.post(url, data=overpass_query)
            response.raise_for_status()
            
            # 保存为临时文件
            osm_file = os.path.join(self.output_path, f"{config['layer_name']}.osm")
            with open(osm_file, 'wb') as f:
                f.write(response.content)
                
            # 转换为GeoPackage
            gpkg_file = os.path.join(self.output_path, f"{config['layer_name']}.gpkg")
            processing.run("quickosm:query", {
                'DATABASE': gpkg_file,
                'OSMFILE': osm_file,
                'LAYER_NAME': config['layer_name'],
                'GEOMETRY_TYPE': config['geometry_type']
            })
            
            # 加载图层
            layer = QgsVectorLayer(gpkg_file + f"|layername={config['layer_name']}", config['layer_name'], "ogr")
            if layer.isValid():
                self.project.addMapLayer(layer)
                
            # 移除临时文件
            os.remove(osm_file)
            
        except Exception as e:
            # 如果下载失败，创建虚拟图层
            QgsMessageLog.logMessage(f"下载{config['layer_name']}失败：{str(e)}，创建虚拟图层", "LushanSuite", Qgis.Warning)
            self._create_dummy_osm_layer(config['layer_name'], config['geometry_type'])
            
    def _create_dummy_osm_layer(self, layer_name, geometry_type):
        """创建虚拟OSM图层（用于测试）"""
        try:
            # 根据几何类型创建图层
            if geometry_type == 'points':
                geom_type = "Point"
            elif geometry_type == 'lines':
                geom_type = "LineString"
            else:
                geom_type = "Polygon"
                
            layer = QgsVectorLayer(
                f"{geom_type}?crs=EPSG:4326", 
                layer_name, 
                "memory"
            )
            
            # 添加字段
            provider = layer.dataProvider()
            provider.addAttributes([
                QgsField("osm_id", QVariant.String),
                QgsField("fclass", QVariant.String),
                QgsField("name", QVariant.String)
            ])
            layer.updateFields()
            
            # 添加到项目
            self.project.addMapLayer(layer)
            
        except Exception as e:
            QgsMessageLog.logMessage(f"创建虚拟图层失败：{str(e)}", "LushanSuite", Qgis.Warning)
            
    def reproject_layers(self):
        """投影转换所有图层"""
        try:
            self.statusChanged.emit("process", "开始投影转换...")
            
            # 获取需要投影的图层
            layers_to_reproject = [
                "交通站点&索道", "山峰", "道路", "河流", "湖泊", "土地利用范围"
            ]
            
            for i, layer_name in enumerate(layers_to_reproject):
                self.statusChanged.emit("process", f"投影转换: {layer_name}")
                layer = self._get_layer_by_name(layer_name)
                if layer and layer.crs() != self.target_crs:
                    self._reproject_layer(layer)
                self.progressChanged.emit(int((i+1)/len(layers_to_reproject)*100))
                    
            self.statusChanged.emit("process", "投影转换完成")
            return True
            
        except Exception as e:
            self.statusChanged.emit("process", f"投影转换失败：{str(e)}")
            return False
            
    def _reproject_layer(self, layer):
        """投影转换单个图层"""
        try:
            # 创建临时文件
            temp_file = os.path.join(self.output_path, f"reprojected_{layer.name()}.gpkg")
            
            # 执行投影转换
            processing.run("native:reprojectlayer", {
                'INPUT': layer,
                'TARGET_CRS': self.target_crs,
                'OUTPUT': temp_file
            })
            
            # 加载投影后的图层
            reprojected_layer = QgsVectorLayer(temp_file, f"{layer.name()}_投影", "ogr")
            if reprojected_layer.isValid():
                self.project.addMapLayer(reprojected_layer)
                # 移除原图层
                self.project.removeMapLayer(layer)
                
        except Exception as e:
            QgsMessageLog.logMessage(f"投影图层{layer.name()}失败：{str(e)}", "LushanSuite", Qgis.Warning)
            
    def symbolize_layers(self):
        """符号化所有图层"""
        try:
            self.statusChanged.emit("process", "开始符号化...")
            
            # 符号化各个图层
            self._symbolize_transport_layer()
            self._symbolize_peak_layer()
            self._symbolize_road_layer()
            self._symbolize_river_layer()
            self._symbolize_lake_layer()
            self._symbolize_landuse_layer()
            
            self.statusChanged.emit("process", "符号化完成")
            return True
            
        except Exception as e:
            self.statusChanged.emit("process", f"符号化失败：{str(e)}")
            return False
            
    def _symbolize_transport_layer(self):
        """符号化交通站点图层"""
        try:
            layer = self._get_layer_by_name("交通站点&索道")
            if not layer:
                return
                
            # 创建分类渲染器
            categories = []
            
            # 火车站
            railway_symbol = QgsMarkerSymbol.createSimple({
                'name': 'triangle',
                'size': '3.5',
                'size_unit': 'MM',
                'color': '#ff0000'
            })
            categories.append(QgsRendererCategory('railway_station', railway_symbol, '火车站'))
            
            # 公交站
            bus_stop_symbol = QgsMarkerSymbol.createSimple({
                'name': 'circle',
                'size': '3.5',
                'size_unit': 'MM',
                'color': '#0000ff'
            })
            categories.append(QgsRendererCategory('bus_stop', bus_stop_symbol, '公交站'))
            
            # 客运站
            bus_station_symbol = QgsMarkerSymbol.createSimple({
                'name': 'square',
                'size': '3.3',
                'size_unit': 'MM',
                'color': '#00ff00'
            })
            categories.append(QgsRendererCategory('bus_station', bus_station_symbol, '客运站'))
            
            # 应用分类渲染器
            renderer = QgsCategorizedSymbolRenderer('fclass', categories)
            layer.setRenderer(renderer)
            layer.triggerRepaint()
            
        except Exception as e:
            QgsMessageLog.logMessage(f"符号化交通站点图层失败：{str(e)}", "LushanSuite", Qgis.Warning)
            
    def _symbolize_peak_layer(self):
        """符号化山峰图层"""
        try:
            layer = self._get_layer_by_name("山峰")
            if not layer:
                return
                
            symbol = QgsMarkerSymbol.createSimple({
                'name': 'triangle',
                'size': '2',
                'size_unit': 'MM',
                'color': 'black'
            })
            
            layer.renderer().setSymbol(symbol)
            layer.triggerRepaint()
            
        except Exception as e:
            QgsMessageLog.logMessage(f"符号化山峰图层失败：{str(e)}", "LushanSuite", Qgis.Warning)
            
    def _symbolize_road_layer(self):
        """符号化道路图层"""
        try:
            layer = self._get_layer_by_name("道路")
            if not layer:
                return
                
            # 创建分类渲染器
            categories = []
            
            # 主要道路
            primary_symbol = QgsLineSymbol.createSimple({
                'color': '#000000',
                'width': '1.0',
                'width_unit': 'MM'
            })
            categories.append(QgsRendererCategory('primary', primary_symbol, '主要道路'))
            
            # 次要道路
            secondary_symbol = QgsLineSymbol.createSimple({
                'color': '#333333',
                'width': '0.8',
                'width_unit': 'MM'
            })
            categories.append(QgsRendererCategory('secondary', secondary_symbol, '次要道路'))
            
            # 三级道路
            tertiary_symbol = QgsLineSymbol.createSimple({
                'color': '#666666',
                'width': '0.6',
                'width_unit': 'MM'
            })
            categories.append(QgsRendererCategory('tertiary', tertiary_symbol, '三级道路'))
            
            # 住宅道路
            residential_symbol = QgsLineSymbol.createSimple({
                'color': '#999999',
                'width': '0.4',
                'width_unit': 'MM'
            })
            categories.append(QgsRendererCategory('residential', residential_symbol, '住宅道路'))
            categories.append(QgsRendererCategory('unclassified', residential_symbol, '未分类道路'))
            
            # 服务道路
            service_symbol = QgsLineSymbol.createSimple({
                'color': '#cccccc',
                'width': '0.3',
                'width_unit': 'MM'
            })
            categories.append(QgsRendererCategory('service', service_symbol, '服务道路'))
            categories.append(QgsRendererCategory('track', service_symbol, '轨道'))
            
            # 小路
            footway_symbol = QgsLineSymbol.createSimple({
                'color': '#dddddd',
                'width': '0.2',
                'width_unit': 'MM',
                'style': 'dash'
            })
            categories.append(QgsRendererCategory('footway', footway_symbol, '小路'))
            categories.append(QgsRendererCategory('path', footway_symbol, '路径'))
            categories.append(QgsRendererCategory('pedestrian', footway_symbol, '人行道'))
            categories.append(QgsRendererCategory('steps', footway_symbol, '台阶'))
            
            # 应用分类渲染器
            renderer = QgsCategorizedSymbolRenderer('fclass', categories)
            layer.setRenderer(renderer)
            layer.triggerRepaint()
            
        except Exception as e:
            QgsMessageLog.logMessage(f"符号化道路图层失败：{str(e)}", "LushanSuite", Qgis.Warning)
            
    def _symbolize_river_layer(self):
        """符号化河流图层"""
        try:
            layer = self._get_layer_by_name("河流")
            if not layer:
                return
                
            symbol = QgsLineSymbol.createSimple({
                'color': '#3f9cff',
                'width': '1.0',
                'width_unit': 'MM'
            })
            
            layer.renderer().setSymbol(symbol)
            layer.triggerRepaint()
            
        except Exception as e:
            QgsMessageLog.logMessage(f"符号化河流图层失败：{str(e)}", "LushanSuite", Qgis.Warning)
            
    def _symbolize_lake_layer(self):
        """符号化湖泊图层"""
        try:
            layer = self._get_layer_by_name("湖泊")
            if not layer:
                return
                
            symbol = QgsFillSymbol.createSimple({
                'color': '#4a8dd3',
                'color_border': 'black',
                'width_border': '0.1',
                'width_border_unit': 'MM'
            })
            
            layer.renderer().setSymbol(symbol)
            layer.triggerRepaint()
            
        except Exception as e:
            QgsMessageLog.logMessage(f"符号化湖泊图层失败：{str(e)}", "LushanSuite", Qgis.Warning)
            
    def _symbolize_landuse_layer(self):
        """符号化土地利用图层"""
        try:
            layer = self._get_layer_by_name("土地利用范围")
            if not layer:
                return
                
            symbol = QgsFillSymbol.createSimple({
                'color': '#bac7b8',
                'color_border': '#c3c3c3',
                'width_border': '0.1',
                'width_border_unit': 'MM'
            })
            
            layer.renderer().setSymbol(symbol)
            layer.triggerRepaint()
            
        except Exception as e:
            QgsMessageLog.logMessage(f"符号化土地利用图层失败：{str(e)}", "LushanSuite", Qgis.Warning)
            
    def add_labeling(self):
        """添加文字注记"""
        try:
            self.statusChanged.emit("process", "开始添加注记...")
            
            # 为各个图层添加标注
            self._add_peak_labeling()
            self._add_road_labeling()
            self._add_river_labeling()
            self._add_lake_labeling()
            
            self.statusChanged.emit("process", "注记添加完成")
            return True
            
        except Exception as e:
            self.statusChanged.emit("process", f"添加注记失败：{str(e)}")
            return False
            
    def _add_peak_labeling(self):
        """添加山峰标注"""
        try:
            layer = self._get_layer_by_name("山峰")
            if not layer:
                return
                
            # 创建标注设置
            pal_settings = QgsPalLayerSettings()
            pal_settings.fieldName = "name"
            pal_settings.enabled = True
            
            # 文本格式
            text_format = QgsTextFormat()
            text_format.setFont(QFont("SimSun", 11))
            text_format.setSize(11)
            text_format.setColor(QColor("#825000"))
            
            # 文本描边
            buffer_settings = QgsTextBufferSettings()
            buffer_settings.setEnabled(True)
            buffer_settings.setSize(0.5)
            buffer_settings.setColor(QColor("#ffffff"))
            text_format.setBuffer(buffer_settings)
            
            pal_settings.setFormat(text_format)
            
            # 应用标注
            labeling = QgsVectorLayerSimpleLabeling(pal_settings)
            layer.setLabeling(labeling)
            layer.setLabelsEnabled(True)
            
        except Exception as e:
            QgsMessageLog.logMessage(f"添加山峰标注失败：{str(e)}", "LushanSuite", Qgis.Warning)
            
    def _add_road_labeling(self):
        """添加道路标注"""
        try:
            layer = self._get_layer_by_name("道路")
            if not layer:
                return
                
            # 创建标注设置
            pal_settings = QgsPalLayerSettings()
            pal_settings.fieldName = "name"
            pal_settings.enabled = True
            
            # 文本格式
            text_format = QgsTextFormat()
            text_format.setFont(QFont("SimSun", 8))
            text_format.setSize(8)
            text_format.setColor(QColor("black"))
            
            pal_settings.setFormat(text_format)
            pal_settings.placement = QgsPalLayerSettings.Line
            
            # 应用标注
            labeling = QgsVectorLayerSimpleLabeling(pal_settings)
            layer.setLabeling(labeling)
            layer.setLabelsEnabled(True)
            
        except Exception as e:
            QgsMessageLog.logMessage(f"添加道路标注失败：{str(e)}", "LushanSuite", Qgis.Warning)
            
    def _add_river_labeling(self):
        """添加河流标注"""
        try:
            layer = self._get_layer_by_name("河流")
            if not layer:
                return
                
            pal_settings = QgsPalLayerSettings()
            pal_settings.fieldName = "name"
            pal_settings.enabled = True
            
            # 文本格式
            text_format = QgsTextFormat()
            text_format.setFont(QFont("SimSun", 6.5))
            text_format.setSize(6.5)
            text_format.setColor(QColor("#0000ca"))
            
            # 文本描边
            buffer_settings = QgsTextBufferSettings()
            buffer_settings.setEnabled(True)
            buffer_settings.setSize(0.5)
            buffer_settings.setColor(QColor("#ffffff"))
            text_format.setBuffer(buffer_settings)
            
            pal_settings.setFormat(text_format)
            pal_settings.placement = QgsPalLayerSettings.Line
            
            # 应用标注
            labeling = QgsVectorLayerSimpleLabeling(pal_settings)
            layer.setLabeling(labeling)
            layer.setLabelsEnabled(True)
            
        except Exception as e:
            QgsMessageLog.logMessage(f"添加河流标注失败：{str(e)}", "LushanSuite", Qgis.Warning)
            
    def _add_lake_labeling(self):
        """添加湖泊标注"""
        try:
            layer = self._get_layer_by_name("湖泊")
            if not layer:
                return
                
            pal_settings = QgsPalLayerSettings()
            pal_settings.fieldName = "name"
            pal_settings.enabled = True
            
            # 文本格式
            text_format = QgsTextFormat()
            text_format.setFont(QFont("SimSun", 10))
            text_format.setSize(10)
            text_format.setColor(QColor("#0101ff"))
            
            pal_settings.setFormat(text_format)
            
            # 应用标注
            labeling = QgsVectorLayerSimpleLabeling(pal_settings)
            layer.setLabeling(labeling)
            layer.setLabelsEnabled(True)
            
        except Exception as e:
            QgsMessageLog.logMessage(f"添加湖泊标注失败：{str(e)}", "LushanSuite", Qgis.Warning)
            
    def _get_layer_by_name(self, name):
        """根据名称获取图层"""
        layers = self.project.mapLayersByName(name)
        return layers[0] if layers else None