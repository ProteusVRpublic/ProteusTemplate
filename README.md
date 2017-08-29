### Proteus Template

Simple. No plugins. No fuss.

Video at https://www.youtube.com/watch?v=ml2xgiQ41BY 

# Features #
* A heavily-modified version of Epic’s VR Template plus:
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
 * UI interactions, VR keyboard
 * Fade out vision when head goes through objects with VRCollision enabled
 * Skeletal socket use when grabbing objects
 * Vive Tracker tracking
 * Network and single player mode
 * Works single player, or multiplayer via LAN, Steam or Oculus Network
 * Direct IP Connect
 * Oculus Direct Connect
 * VOIP
 * Oculus Avatars

# Latest Versions #

# Version 3.2.2 (August 29, 2017) #
# UNREAL 4.17 VERSION

# Latest changelog #

2 important fixes:
* “NavModifiersVolume (NavAreaNull) have inconsistent behaviour since Unreal 4.16. For now I ditched the NavMesh and teleport on static mesh, deny teleportation with floor angle and CustomNavArea NavModifierVolume, until Navigation is more stable in Unreal.” [NOW FIXED]
* “I've had the same teleportation direction issue through all of the versions of the template I've tested. For me the forward direction seems to be the "right->" direction of my playspace. Any luck reproducing the issue with teleport rotation on the vive? There are times it seems like I'm 90-180 degrees from where I'm supposed to end up and its really confusing sometimes. I haven't been able to figure it out myself » [NOW FIXED]

What I recommend: Try the 4.17 packaged version on the OneDrive (Template 3.2.2). If there is problem there, it’s on your side, probably setup based.

KNOWN BUG WITHOUT A FIX (FOR NOW)
No VOIP through Direct IP Connection: I’ve tried many tricks / solutions, nothing works (yet). Maybe using Advanced Session Plugin? I’ll try that. If someone has a trick for enabling VOIP through direct IP give me a shout.
Physics objects: It’s normal to experience client-side lag on physics objects (i.e. Interactive Objects). Many tricks exist to fix that problem; search for “multiplayer physics objects”. This would be an entire different topic and is not specifically related to VR.
Oculus Avatars Version : Remote Avatar is not controlled by Remote Player : Working on it.
BUGS WITH NO FIX
Affecting ONLY Unreal 4.16.3: UE-45908 Widget interaction debug line only shows when hitting a widget https://issues.unrealengine.com/issue/UE-45908 - Bug has been resolved in 4.17
BUGS REPORTED TO ANSWERHUB
Affecting ONLY version 4.17, and ONLY Oculus Rift: UE-48260 Play area bounds are offset so that the player's spawn position is at the center of the player start https://issues.unrealengine.com/issue/UE-48260

Affecting Unreal 4.16.3 and 4.17.0: UE-44947 Extra Sphere collision is shown when setting it to not be hidden in game for the MotioncontrollerBP
https://issues.unrealengine.com/issue/UE-44947 You can resolve this by unchecking "Instanced Stereo" in Project Settings, but doing that could decrease VR performance.


Please read the pdf document for the complete instructions. It is found at https://1drv.ms/f/s!Av77lIIxt2OY0XGGW8UDwykohjuT  
