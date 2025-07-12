# -*- coding: utf-8 -*-
"""
/***************************************************************************
 LushanMapTools
                                 A QGIS plugin
 Map making tools for Lushan practical training
                              -------------------
        begin                : 2025-07-10
        copyright            : (C) 2025 by Yang
        email                : liyang22@whu.edu.cn
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

import os
import tempfile
from qgis.core import (
    QgsProcessingFeedback,
    QgsProcessingContext,
    QgsCoordinateReferenceSystem,
    QgsRasterLayer,
    QgsVectorLayer,
    QgsProject,
    QgsMapLayerStyle,
    QgsRasterRenderer,
    QgsColorRampShader,
    QgsRasterShader,
    QgsSingleBandPseudoColorRenderer,
    QgsHillshadeRenderer,
    QgsContourRenderer,
    QgsLineSymbol,
    QgsMarkerLineSymbolLayer,
    QgsSimpleLineSymbolLayer,
    QgsRuleBasedLabeling,
    QgsPalLayerSettings,
    QgsTextFormat,
    QgsTextBufferSettings,
    QgsVectorLayerSimpleLabeling,
    QgsGeometry,
    QgsFeature,
    QgsWkbTypes,
    QgsProcessingException,
    QgsField,
    QgsVectorFileWriter
)
from qgis.PyQt.QtCore import QVariant, pyqtSignal, QObject
from qgis.PyQt.QtGui import QColor
import processing


class LushanMapProcessor(QObject):
    """Main processor for Lushan map making operations"""
    
    # Signals for progress reporting
    progressChanged = pyqtSignal(int)
    messageLogged = pyqtSignal(str)
    finished = pyqtSignal(bool, str)
    
    def __init__(self):
        super().__init__()
        self.feedback = None
        self.context = None
        
    def setup_processing_environment(self):
        """Setup QGIS processing environment"""
        self.feedback = QgsProcessingFeedback()
        self.context = QgsProcessingContext()
        
        # Connect feedback signals
        self.feedback.progressChanged.connect(self.progressChanged.emit)
        
    def log_message(self, message):
        """Log a message to the interface"""
        self.messageLogged.emit(message)
        
    def clip_raster_to_extent(self, input_raster, extent_layer, output_path, target_crs="EPSG:32650"):
        """
        Clip DEM raster to the map extent layer and reproject if needed
        
        Args:
            input_raster (str): Path to input DEM file
            extent_layer (QgsVectorLayer): Vector layer defining the clipping extent
            output_path (str): Path for output clipped raster
            target_crs (str): Target coordinate reference system
            
        Returns:
            str: Path to the clipped raster file
        """
        try:
            self.log_message(f"开始裁剪DEM数据: {input_raster}")
            
            # Load the input raster
            raster_layer = QgsRasterLayer(input_raster, "input_dem")
            if not raster_layer.isValid():
                raise QgsProcessingException(f"无法加载DEM文件: {input_raster}")
            
            # Get extent from vector layer
            extent = extent_layer.extent()
            extent_string = f"{extent.xMinimum()},{extent.xMaximum()},{extent.yMinimum()},{extent.yMaximum()}"
            
            # Clip raster
            clip_params = {
                'INPUT': input_raster,
                'MASK': extent_layer,
                'SOURCE_CRS': None,
                'TARGET_CRS': QgsCoordinateReferenceSystem(target_crs),
                'NODATA': None,
                'ALPHA_BAND': False,
                'CROP_TO_CUTLINE': True,
                'KEEP_RESOLUTION': False,
                'SET_RESOLUTION': False,
                'X_RESOLUTION': None,
                'Y_RESOLUTION': None,
                'MULTITHREADING': False,
                'OPTIONS': '',
                'DATA_TYPE': 0,
                'EXTRA': '',
                'OUTPUT': output_path
            }
            
            result = processing.run("gdal:cliprasterbymasklayer", clip_params, 
                                  context=self.context, feedback=self.feedback)
            
            self.log_message(f"DEM裁剪完成: {result['OUTPUT']}")
            return result['OUTPUT']
            
        except Exception as e:
            self.log_message(f"DEM裁剪失败: {str(e)}")
            raise
    
    def create_dem_rendering(self, input_raster, output_path, opacity=80):
        """
        Create DEM rendering with specified color gradient
        
        Args:
            input_raster (str): Path to input DEM raster
            output_path (str): Path for output rendered raster
            opacity (int): Opacity percentage (0-100)
            
        Returns:
            QgsRasterLayer: Styled raster layer
        """
        try:
            self.log_message("创建DEM渲染图...")
            
            # Load the raster
            raster_layer = QgsRasterLayer(input_raster, "dem_render")
            if not raster_layer.isValid():
                raise QgsProcessingException(f"无法加载DEM文件: {input_raster}")
            
            # Get raster statistics for color ramp
            provider = raster_layer.dataProvider()
            stats = provider.bandStatistics(1)
            min_val = stats.minimumValue
            max_val = stats.maximumValue
            
            # Create color ramp shader
            shader = QgsRasterShader()
            color_ramp = QgsColorRampShader()
            color_ramp.setColorRampType(QgsColorRampShader.Interpolated)
            
            # Define color stops based on requirements
            # 0%:#d8e5be, 29%:#add59c, 89%:#4caf50, 100%:#006603
            color_list = [
                QgsColorRampShader.ColorRampItem(min_val, QColor("#d8e5be"), "Low"),
                QgsColorRampShader.ColorRampItem(min_val + (max_val - min_val) * 0.29, QColor("#add59c"), "Low-Mid"),
                QgsColorRampShader.ColorRampItem(min_val + (max_val - min_val) * 0.89, QColor("#4caf50"), "High-Mid"),
                QgsColorRampShader.ColorRampItem(max_val, QColor("#006603"), "High")
            ]
            
            color_ramp.setColorRampItemList(color_list)
            shader.setRasterShaderFunction(color_ramp)
            
            # Create renderer
            renderer = QgsSingleBandPseudoColorRenderer(provider, 1, shader)
            renderer.setOpacity(opacity / 100.0)
            
            # Apply renderer to layer
            raster_layer.setRenderer(renderer)
            raster_layer.triggerRepaint()
            
            # Add to project
            QgsProject.instance().addMapLayer(raster_layer)
            
            self.log_message("DEM渲染图创建完成")
            return raster_layer
            
        except Exception as e:
            self.log_message(f"DEM渲染失败: {str(e)}")
            raise
    
    def create_hillshade(self, input_raster, output_path, z_factor=1.0, azimuth=315, elevation=35):
        """
        Create hillshade from DEM
        
        Args:
            input_raster (str): Path to input DEM raster
            output_path (str): Path for output hillshade raster
            z_factor (float): Z factor for hillshade calculation
            azimuth (int): Light source azimuth angle
            elevation (int): Light source elevation angle
            
        Returns:
            str: Path to the hillshade file
        """
        try:
            self.log_message("创建山体阴影图...")
            
            hillshade_params = {
                'INPUT': input_raster,
                'BAND': 1,
                'Z_FACTOR': z_factor,
                'SCALE': 1,
                'AZIMUTH': azimuth,
                'V_ANGLE': elevation,
                'COMPUTE_EDGES': False,
                'ZEVENBERGEN': False,
                'OPTIONS': '',
                'EXTRA': '',
                'OUTPUT': output_path
            }
            
            result = processing.run("gdal:hillshade", hillshade_params,
                                  context=self.context, feedback=self.feedback)
            
            # Load and style the hillshade
            hillshade_layer = QgsRasterLayer(result['OUTPUT'], "hillshade")
            if hillshade_layer.isValid():
                # Set grayscale rendering for hillshade
                renderer = hillshade_layer.renderer().clone()
                renderer.setOpacity(0.7)  # Set transparency
                hillshade_layer.setRenderer(renderer)
                
                # Add to project (should be below DEM layer)
                QgsProject.instance().addMapLayer(hillshade_layer, False)
                root = QgsProject.instance().layerTreeRoot()
                root.insertLayer(0, hillshade_layer)  # Insert at bottom
            
            self.log_message(f"山体阴影图创建完成: {result['OUTPUT']}")
            return result['OUTPUT']
            
        except Exception as e:
            self.log_message(f"山体阴影图创建失败: {str(e)}")
            raise
    
    def create_contours(self, input_raster, output_dir, interval_100m=True, interval_20m=True,
                       color_100m="#fffac1", width_100m=0.3, opacity_100m=60, labels_100m=True,
                       color_20m="#fff5a5", width_20m=0.1, opacity_20m=50, labels_20m=False):
        """
        Create contour lines from DEM
        
        Args:
            input_raster (str): Path to input DEM raster
            output_dir (str): Directory for output contour files
            interval_100m (bool): Create 100m contours
            interval_20m (bool): Create 20m contours
            color_100m (str): Color for 100m contours
            width_100m (float): Line width for 100m contours (mm)
            opacity_100m (int): Opacity for 100m contours (0-100)
            labels_100m (bool): Show labels for 100m contours
            color_20m (str): Color for 20m contours
            width_20m (float): Line width for 20m contours (mm)
            opacity_20m (int): Opacity for 20m contours (0-100)
            labels_20m (bool): Show labels for 20m contours
            
        Returns:
            list: List of created contour layer paths
        """
        try:
            created_layers = []
            
            if interval_100m:
                self.log_message("创建100m等值线...")
                contour_100_path = os.path.join(output_dir, "contours_100m.shp")
                
                contour_params = {
                    'INPUT': input_raster,
                    'BAND': 1,
                    'INTERVAL': 100,
                    'FIELD_NAME': 'ELEV',
                    'CREATE_3D': False,
                    'IGNORE_NODATA': False,
                    'NODATA': None,
                    'OFFSET': 0,
                    'EXTRA': '',
                    'OUTPUT': contour_100_path
                }
                
                result = processing.run("gdal:contour", contour_params,
                                      context=self.context, feedback=self.feedback)
                
                # Style the 100m contours
                contour_layer = QgsVectorLayer(result['OUTPUT'], "Contours_100m", "ogr")
                if contour_layer.isValid():
                    self._style_contour_layer(contour_layer, color_100m, width_100m, 
                                            opacity_100m, labels_100m, "100m等值线")
                    QgsProject.instance().addMapLayer(contour_layer)
                    created_layers.append(result['OUTPUT'])
                
                self.log_message("100m等值线创建完成")
            
            if interval_20m:
                self.log_message("创建20m等值线...")
                contour_20_path = os.path.join(output_dir, "contours_20m.shp")
                
                contour_params = {
                    'INPUT': input_raster,
                    'BAND': 1,
                    'INTERVAL': 20,
                    'FIELD_NAME': 'ELEV',
                    'CREATE_3D': False,
                    'IGNORE_NODATA': False,
                    'NODATA': None,
                    'OFFSET': 0,
                    'EXTRA': '',
                    'OUTPUT': contour_20_path
                }
                
                result = processing.run("gdal:contour", contour_params,
                                      context=self.context, feedback=self.feedback)
                
                # Style the 20m contours
                contour_layer = QgsVectorLayer(result['OUTPUT'], "Contours_20m", "ogr")
                if contour_layer.isValid():
                    self._style_contour_layer(contour_layer, color_20m, width_20m, 
                                            opacity_20m, labels_20m, "20m等值线")
                    QgsProject.instance().addMapLayer(contour_layer)
                    created_layers.append(result['OUTPUT'])
                
                self.log_message("20m等值线创建完成")
            
            return created_layers
            
        except Exception as e:
            self.log_message(f"等值线创建失败: {str(e)}")
            raise
    
    def _style_contour_layer(self, layer, color, width_mm, opacity, show_labels, layer_name):
        """Style a contour layer with specified parameters"""
        try:
            # Create line symbol
            symbol = QgsLineSymbol.createSimple({
                'color': color,
                'width': str(width_mm),
                'width_unit': 'MM',
                'opacity': str(opacity / 100.0)
            })
            
            # Apply symbol to layer
            layer.renderer().setSymbol(symbol)
            
            # Setup labeling if requested
            if show_labels:
                # Create label settings
                label_settings = QgsPalLayerSettings()
                label_settings.fieldName = 'ELEV'
                label_settings.enabled = True
                
                # Text format
                text_format = QgsTextFormat()
                text_format.setFont(layer.renderer().symbol().symbolLayer(0).properties().get('font', 'SimSun'))
                text_format.setSize(0.6)  # 0.6 points as specified
                text_format.setColor(QColor("black"))
                
                # Text buffer for better visibility
                buffer_settings = QgsTextBufferSettings()
                buffer_settings.setEnabled(True)
                buffer_settings.setSize(0.2)
                buffer_settings.setColor(QColor("white"))
                text_format.setBuffer(buffer_settings)
                
                label_settings.setFormat(text_format)
                
                # Parallel placement along lines, middle position
                label_settings.placement = QgsPalLayerSettings.Line
                label_settings.lineSettings().setPlacementFlags(QgsPalLayerSettings.AboveLine)
                
                # Apply labeling
                labeling = QgsVectorLayerSimpleLabeling(label_settings)
                layer.setLabeling(labeling)
                layer.setLabelsEnabled(True)
            
            # Set layer name
            layer.setName(layer_name)
            
        except Exception as e:
            self.log_message(f"图层样式设置失败: {str(e)}")
    
    def create_map_extent_layer(self, geometry, output_path, crs="EPSG:32650"):
        """
        Create a map extent layer from geometry
        
        Args:
            geometry (QgsGeometry): Polygon geometry for map extent
            output_path (str): Path for output shapefile
            crs (str): Coordinate reference system
            
        Returns:
            QgsVectorLayer: Created extent layer
        """
        try:
            self.log_message("创建图廓层...")
            
            # Create vector layer
            extent_layer = QgsVectorLayer(f"Polygon?crs={crs}", "Map_Extent", "memory")
            
            # Add fields
            provider = extent_layer.dataProvider()
            provider.addAttributes([
                QgsField("id", QVariant.Int),
                QgsField("name", QVariant.String)
            ])
            extent_layer.updateFields()
            
            # Create feature
            feature = QgsFeature()
            feature.setGeometry(geometry)
            feature.setAttributes([1, "Map Extent"])
            
            # Add feature to layer
            provider.addFeatures([feature])
            extent_layer.updateExtents()
            
            # Style the layer
            symbol = extent_layer.renderer().symbol()
            symbol.setColor(QColor(255, 0, 0, 50))  # Semi-transparent red fill
            symbol.symbolLayer(0).setStrokeColor(QColor(255, 0, 0))  # Red outline
            symbol.symbolLayer(0).setStrokeWidth(2)
            
            # Save to file if output path provided
            if output_path:
                options = QgsVectorFileWriter.SaveVectorOptions()
                options.driverName = "ESRI Shapefile"
                options.fileEncoding = "UTF-8"
                
                error = QgsVectorFileWriter.writeAsVectorFormat(
                    extent_layer, output_path, options
                )
                
                if error[0] != QgsVectorFileWriter.NoError:
                    raise QgsProcessingException(f"无法保存图廓层: {error[1]}")
            
            # Add to project
            QgsProject.instance().addMapLayer(extent_layer)
            
            self.log_message("图廓层创建完成")
            return extent_layer
            
        except Exception as e:
            self.log_message(f"图廓层创建失败: {str(e)}")
            raise
    
    def process_complete_workflow(self, dem_path, extent_layer, output_dir, 
                                 dem_opacity=80, hillshade_params=None, contour_params=None):
        """
        Execute the complete map making workflow
        
        Args:
            dem_path (str): Path to input DEM file
            extent_layer (QgsVectorLayer): Map extent layer
            output_dir (str): Output directory
            dem_opacity (int): DEM rendering opacity
            hillshade_params (dict): Hillshade parameters
            contour_params (dict): Contour parameters
            
        Returns:
            dict: Results of the processing
        """
        try:
            self.setup_processing_environment()
            
            # Default parameters
            if hillshade_params is None:
                hillshade_params = {'z_factor': 1.0, 'azimuth': 315, 'elevation': 35}
            
            if contour_params is None:
                contour_params = {
                    'interval_100m': True, 'interval_20m': True,
                    'color_100m': '#fffac1', 'width_100m': 0.3, 'opacity_100m': 60, 'labels_100m': True,
                    'color_20m': '#fff5a5', 'width_20m': 0.1, 'opacity_20m': 50, 'labels_20m': False
                }
            
            results = {}
            
            # Step 1: Clip DEM to extent
            self.progressChanged.emit(10)
            clipped_dem_path = os.path.join(output_dir, "dem_clipped.tif")
            results['clipped_dem'] = self.clip_raster_to_extent(
                dem_path, extent_layer, clipped_dem_path
            )
            
            # Step 2: Create DEM rendering
            self.progressChanged.emit(30)
            dem_render_path = os.path.join(output_dir, "dem.tif")
            results['dem_render'] = self.create_dem_rendering(
                results['clipped_dem'], dem_render_path, dem_opacity
            )
            
            # Step 3: Create hillshade
            self.progressChanged.emit(50)
            hillshade_path = os.path.join(output_dir, "shade.tif")
            results['hillshade'] = self.create_hillshade(
                results['clipped_dem'], hillshade_path, **hillshade_params
            )
            
            # Step 4: Create contours
            self.progressChanged.emit(70)
            results['contours'] = self.create_contours(
                results['clipped_dem'], output_dir, **contour_params
            )
            
            self.progressChanged.emit(100)
            self.log_message("所有处理步骤完成！")
            self.finished.emit(True, "处理成功完成")
            
            return results
            
        except Exception as e:
            error_msg = f"处理失败: {str(e)}"
            self.log_message(error_msg)
            self.finished.emit(False, error_msg)
            raise


class MapExtentTool(QObject):
    """Tool for creating map extent polygons interactively"""
    
    def __init__(self, iface):
        super().__init__()
        self.iface = iface
        self.canvas = iface.mapCanvas()
        
    def start_extent_creation(self):
        """Start interactive extent creation"""
        # This would typically involve a map tool for drawing polygons
        # For now, we'll use the current map canvas extent
        extent = self.canvas.extent()
        
        # Create rectangle geometry from extent
        geometry = QgsGeometry.fromRect(extent)
        
        return geometry