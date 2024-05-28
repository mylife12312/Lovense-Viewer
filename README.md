<picture>
  <img alt="Lovense Viewer Logo" src="doc/login_viewer_logo.png">
</picture>

**The [Lovense Viewer](https://www.lovense.com/game/lovense-viewer/) is a free third-party viewer for Second Life that enables seamless integration with Lovense products.** Connect your favorite Lovense toy(s) to the Lovense Viewer and explore the captivating world of Second Life with enhanced interactivity and immersive experiences. This repository contains the official source code for the Lovense viewer.

## Open Source

Lovense viewer is a third party viewer derived from the official [Second Life](https://github.com/secondlife/viewer) client. The client codebase has been open source since 2007 and is available under the LGPL license.

## Download

Pre-built versions of the viewer releases for Windows and Mac can be downloaded from the [official website](https://www.lovense.com/game/lovense-viewer/).

## Build Instructions

### Build Instructions For Windows
##### Prerequisites
- Python 3.0 or later
- Visual Studio 2022
- Cmake 3.0 or later

------------


###### Compilation steps
1.Install autobuild  
`pip install autobuild`  
2.Install llbase     
`pip install llbase`  
3.Use the command line to generate project files    
`autobuild configure -c RelWithDebInfoOS -A 64`  

### Build Instructions For Mac
##### Prerequisites
- Python 3.0 or later
- Xcode 15.0 or later
- Cmake 3.0 or later

------------


###### Compilation steps
1.Install autobuild  
`pip install autobuild`    
2.Use the command line to generate project files    
`autobuild configure -c RelWithDebInfoOS`  
Or Use build script  
`build-xcode.sh`

If you have any questions about compiling please contact developer@mail.lovense.com.

## Contribute

Your advice is precious and makes the Lovense Viewer better. Please feel free to get involved with improvements by sharing thoughts/bugs/suggestions via [Discord](https://discord.gg/lovense) or developer@mail.lovense.com.
