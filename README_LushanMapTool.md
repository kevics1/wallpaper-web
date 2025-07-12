# 庐山实习地图制作工具 (Lushan Practical Map Making Tool)

## 概述 (Overview)

庐山实习地图制作工具是一个功能完善的QGIS插件，专为地理学野外实习和制图需求设计。该插件提供了现代化的用户界面和强大的DEM数据处理功能，能够自动化完成地图制作的核心流程。

The Lushan Practical Map Making Tool is a comprehensive QGIS plugin designed for geography field studies and cartographic needs. It provides a modern user interface and powerful DEM data processing capabilities to automate core map-making workflows.

## 主要功能 (Key Features)

### 1. 定义图廓层功能 (Map Extent Layer Definition)
- **交互式图廓创建**: 允许用户创建制图范围区域面数据
- **现有图层选择**: 支持选择项目中已有的面图层作为图廓层
- **投影支持**: 默认使用EPSG:32650投影，可自定义选择其他投影

### 2. DEM数据处理功能 (DEM Data Processing)
- **自动裁剪**: 基于图廓层自动裁剪DEM数据
- **投影转换**: 自动转换为目标投影坐标系
- **智能渲染**: 采用预设颜色梯度进行DEM可视化

### 3. 晕渲图底图制作 (Hillshade Base Map Creation)
- **山体阴影生成**: 可配置Z因子、方位角、垂直角参数
- **图层叠加**: 自动将晕渲图置于DEM图层下方
- **透明度控制**: 支持自定义透明度设置

### 4. 等值线制作功能 (Contour Line Generation)
- **多间距等值线**: 支持100m和20m间距等值线
- **样式定制**: 可配置颜色、线宽、透明度
- **智能标注**: 支持等值线高程标注，可自定义字体和位置

### 5. 现代化用户界面 (Modern User Interface)
- **分组布局**: 清晰的功能分区和步骤指引
- **实时反馈**: 进度条和详细日志显示
- **参数配置**: 丰富的参数自定义选项
- **错误处理**: 完善的输入验证和错误提示

## 文件结构 (File Structure)

```
lushan_efficiency_suite/
├── lushan_efficiency_suite.py              # 主插件文件
├── lushan_efficiency_suite_dialog.py       # 对话框逻辑
├── lushan_efficiency_suite_dialog_base.ui  # UI界面设计
├── lushan_map_tools.py                     # 地图处理工具模块
├── resources.py                            # 资源文件
├── resources.qrc                           # Qt资源配置
└── __init__.py                             # 插件初始化
```

## 技术架构 (Technical Architecture)

### 核心组件 (Core Components)

1. **LushanEfficiencySuite**: 主插件类，负责QGIS集成
2. **LushanEfficiencySuiteDialog**: 用户界面对话框
3. **LushanMapProcessor**: 核心地图处理引擎
4. **MapExtentTool**: 图廓创建工具
5. **ProcessingThread**: 后台处理线程

### 处理流程 (Processing Workflow)

```
1. 用户输入验证 → 2. DEM数据裁剪 → 3. 投影转换 → 4. DEM渲染 → 5. 晕渲图生成 → 6. 等值线创建 → 7. 图层样式应用
```

## 使用方法 (Usage Instructions)

### 基本使用流程 (Basic Workflow)

1. **启动插件**
   - 在QGIS中找到"庐山实习绘图&分析插件"
   - 点击工具栏图标或菜单项启动

2. **定义图廓层**
   - 点击"创建图廓区域"按钮创建新的图廓范围
   - 或点击"选择现有图层"选择已有的面图层

3. **选择DEM数据**
   - 点击"浏览..."按钮选择DEM数据文件
   - 支持常见栅格格式：TIFF, IMG, BIL, ASC等

4. **配置处理参数**
   - **DEM渲染**: 调整不透明度，查看颜色梯度预览
   - **山体阴影**: 设置Z因子、方位角、垂直角
   - **等值线**: 配置颜色、线宽、透明度、标注选项

5. **设置输出目录**
   - 选择处理结果的保存位置

6. **开始处理**
   - 点击"开始处理"按钮启动完整工作流程
   - 实时查看进度条和处理日志

### 参数说明 (Parameter Description)

#### DEM渲染参数
- **不透明度**: 0-100%，控制DEM图层透明度
- **颜色梯度**: 预设四色梯度 (#d8e5be → #add59c → #4caf50 → #006603)

#### 山体阴影参数
- **Z因子**: 1.0（默认），高程夸张系数
- **方位角**: 315°（默认），光源方位角
- **垂直角**: 35°（默认），光源垂直角

#### 等值线参数

**100m等值线**:
- 颜色: #fffac1
- 线宽: 0.3mm
- 不透明度: 60%
- 标注: 宋体，0.6点，黑色，平行模式，线中间

**20m等值线**:
- 颜色: #fff5a5
- 线宽: 0.1mm
- 不透明度: 50%
- 标注: 默认关闭

## 输出结果 (Output Results)

处理完成后将生成以下文件和图层：

1. **dem_clipped.tif**: 裁剪后的DEM数据
2. **dem.tif**: 渲染后的DEM图层
3. **shade.tif**: 山体阴影图
4. **contours_100m.shp**: 100m间距等值线
5. **contours_20m.shp**: 20m间距等值线

所有图层将自动添加到QGIS项目中，并应用相应的样式设置。

## 系统要求 (System Requirements)

- **QGIS版本**: 3.0 或更高版本
- **Python版本**: 3.6 或更高版本
- **依赖模块**: PyQt5, QGIS Processing
- **操作系统**: Windows, macOS, Linux

## 错误处理 (Error Handling)

插件包含完善的错误处理机制：

- **输入验证**: 检查文件路径、图层选择、参数合理性
- **处理异常**: 捕获并报告处理过程中的错误
- **用户反馈**: 友好的错误消息和解决建议
- **日志记录**: 详细的操作日志便于问题诊断

## 开发信息 (Development Information)

- **开发者**: Yang
- **邮箱**: liyang22@whu.edu.cn
- **开始时间**: 2025-07-10
- **许可证**: GNU General Public License v2.0

## 技术实现细节 (Technical Implementation Details)

### 核心算法 (Core Algorithms)

1. **DEM裁剪算法**
   ```python
   processing.run("gdal:cliprasterbymasklayer", parameters)
   ```

2. **山体阴影算法**
   ```python
   processing.run("gdal:hillshade", parameters)
   ```

3. **等值线生成算法**
   ```python
   processing.run("gdal:contour", parameters)
   ```

### 多线程处理 (Multi-threading)

- 使用`QThread`进行后台处理，避免界面冻结
- 实时进度更新和消息传递
- 安全的线程间通信机制

### 内存管理 (Memory Management)

- 智能的临时文件管理
- 大数据集的分块处理
- 及时释放不需要的资源

## 扩展开发 (Extension Development)

### 添加新功能模块

1. **继承基础类**
   ```python
   class NewProcessor(LushanMapProcessor):
       def new_function(self):
           # 实现新功能
           pass
   ```

2. **扩展UI界面**
   - 在UI文件中添加新的控件组
   - 在对话框类中添加相应的信号槽连接

3. **集成处理流程**
   - 将新功能集成到主处理工作流程中
   - 添加相应的参数验证和错误处理

### 自定义样式 (Custom Styling)

可以通过修改样式参数来自定义地图外观：

```python
# 自定义DEM颜色梯度
custom_colors = [
    (min_val, QColor("#custom1")),
    (mid_val, QColor("#custom2")),
    (max_val, QColor("#custom3"))
]
```

## 性能优化建议 (Performance Optimization Tips)

1. **数据预处理**: 对大型DEM数据进行预处理和分块
2. **投影统一**: 确保所有数据使用相同的投影坐标系
3. **内存监控**: 监控内存使用情况，及时释放临时数据
4. **并行处理**: 对独立的处理步骤启用并行计算

## 故障排除 (Troubleshooting)

### 常见问题及解决方案

1. **DEM文件无法读取**
   - 检查文件格式是否受支持
   - 确认文件路径没有中文字符
   - 验证文件是否损坏

2. **投影转换失败**
   - 检查源数据投影信息
   - 确认目标投影EPSG代码正确
   - 验证QGIS投影数据库完整性

3. **内存不足错误**
   - 减小处理区域范围
   - 降低输出分辨率
   - 关闭其他占用内存的程序

4. **处理速度慢**
   - 使用更快的存储设备
   - 减少不必要的中间文件输出
   - 优化处理参数设置

## 更新日志 (Changelog)

### v1.0.0 (2025-07-10)
- 初始版本发布
- 实现完整的地图制作工作流程
- 现代化用户界面设计
- 多线程处理支持
- 完善的错误处理机制

## 联系支持 (Contact & Support)

如有问题或建议，请联系：
- 邮箱: liyang22@whu.edu.cn
- 项目地址: 请查看插件安装目录

---

*本插件基于GNU General Public License v2.0开源协议发布。*