# Lushan Efficiency Suite QGIS Plugin

A QGIS plugin designed to accelerate geospatial workflows for field geography studies, inspired by the Lushan Mountain practicum.

## Features

### Buffer Analysis Tool

The enhanced Buffer Analysis Tool provides comprehensive buffer generation capabilities with an intuitive user interface.

#### Key Features

- **Multi-format Vector Input**: Supports various vector formats including:
  - Shapefile (.shp)
  - GeoPackage (.gpkg) 
  - GeoJSON (.geojson)
  - KML (.kml)
  - GML (.gml)

- **Flexible Buffer Parameters**:
  - Distance input with multiple units (meters, kilometers, feet, miles)
  - Segments control (4-100) for buffer edge smoothness
  - Dissolve option to merge overlapping buffers into single features

- **Advanced Output Options**:
  - Custom output layer naming
  - Option to automatically add results to map canvas
  - Progress tracking during processing

#### How to Use

1. **Launch the Plugin**: 
   - Open QGIS
   - Navigate to Plugins menu > Lushan Efficiency Suite > 庐山实习绘图&分析插件

2. **Select Input Layer**:
   - Click the "Browse..." button to select your vector file
   - The plugin will validate the layer and display information about geometry type, feature count, and CRS

3. **Configure Buffer Parameters**:
   - Set buffer distance and select appropriate units
   - Adjust segments value for smoother curves (higher values = smoother)
   - Check "Dissolve overlapping buffers" if you want merged results

4. **Set Output Options**:
   - Enter a name for the output layer
   - Choose whether to add the result to the map canvas

5. **Execute**:
   - Click "Preview" to see a summary of your settings
   - Click "OK" to run the buffer analysis

#### Technical Details

- Uses QGIS native processing algorithms for optimal performance
- Automatic unit conversion to meters for processing
- Comprehensive error handling and validation
- Memory-efficient processing with temporary layer management

## Installation

1. Copy the plugin folder to your QGIS plugins directory
2. Enable the plugin in QGIS Plugin Manager
3. The plugin will appear in the Plugins menu

## Requirements

- QGIS 3.x
- PyQt5
- Valid vector data in supported formats

## Support

For issues or feature requests, please contact the development team.

## License

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.