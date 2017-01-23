
# Proteus Template

Simple. No plugins. No fuss.

Video at https://www.youtube.com/watch?v=ml2xgiQ41BY

## Features

A heavily-modified version of Epic’s VR Template plus:

* 3 fully animated controllers meshes: Hands (like Oculus Avatar), Oculus Touch controllers and HTC Vive controllers

  * Can be controlled with Oculus Touch or HTC Vive controllers, and support for Oculus remote

  * Full input mapping


* Control over controllers opacity, scale, HTC Vive controllers skins, Avatar color

* For the hands, the scale is 1:1 with Epic VR template. Reduce scale to approx. 0.75 to come close to real-life

* Function to spawn poles at the 4 corners of the Oculus Guardian or SteamVR chaperone limits

  * By default it is mapped to Oculus Touch B and Y buttons, and HTC Vive controllers menu button


* Optimizations for Oculus Rift and HTC Vive HMDs

* “Ghost Mode” with gamepad

  * By default it is turned on; can be turned off or mapped to another input


* Monitor in front of character

  * Can be spawned (turned off by default) to print useful infos / debug


* Avatar head

* Rumble functions adapted to work with Touch & Vive

* UI interactions

* Network and single player mode

  * Works via LAN, Steam an Oculus networks (version 1.1)

  * If Oculus Network is used, Oculus Avatar will be used if present (version 1.2)

    * If not, default Avatar is used

## Version 1.0 (Jan 13, 2017)
### SINGLE PLAYER MODE

#### How does it work?
Use it directly or import it to another project, by migrating the ProteusSingle folder into the project
1.	Set the desired settings in Settings structure file
2.	Set VR_GameMode as desired gamemode
3.	In settings/General Settings/Framerate
4.	In settings/Rendering

### So, what this is about?
This template consists of HTC Vive and Oculus Rift compatible pawn and settings, ready to drop in your game. So what you can do is choose the features you want, and build yourself/modify a pawn based on the functionalities you need.
Source material

* Epic VR template

* Epic Couch Knights (for avatar head)

* All other meshes, animations and blueprints are from Proteus

### Which HMDs work with the template?
The template is optimized for the HTC Vive and Oculus Rift with Vive or Touch controllers w/wo gamepad
Which Unreal Engine 4 version works with the template?
The template has been developed and tested with Unreal Engine 4.14.3
Another VR Template? There’s already a ton of it.
This template is very simple, no plugins, no fuss. It gives a good start to use Touch and Vive controllers, in single or multiplayer mode.

### How can I install it?
Files can be found at https://1drv.ms/f/s!Av77lIIxt2OY0XGGW8UDwykohjuT  
GitHub version at https://github.com/ProteusVR/ProteusTemplate  (you need to be logged to Github to open the link)
Main infos found in the forum at https://forums.unrealengine.com/showthread.php?133957-Single-Multiplayer-Touch-amp-Vive-Proteus-blueprint-only-Template
To install as a template, just unzip into the appropriate templates directory like C:\Program Files\Unreal Engine[Version]\Templates for launcher version or[ForkLocation]\UE4\Templates for source version. Launch a new project, and you'll find it in the blueprint section.
To install as a project file, unzip in your usual projects folder. Then, delete the file /Config/TemplateDefs.ini and you’re ready to go.
IMPORTANT: If you open it like a regular project without deleting the .ini file, you’ll get errors messages.

Please read the pdf document for the complete instructions.
