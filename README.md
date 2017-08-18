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

# Version 3.2.1 (August 18, 2017) #

# Latest changelog #
I’ve had a dozen reports about “What the hell, can’t see my hands!”. ALL these problems were due to Guardian or Chaperone not correctly set during Rift/SteamVR setup. After each person redid their setup, the hands magically went back. So now I’ve put a safeguard for that see fix #1.

What I recommend: Try the 4.17 packaged version on the OneDrive (Template 3.2.1). If there is problem there, it’s on your side, probably setup based.

NEW FEATURE:

Spectator mode toggle (4.17 only)
I put a simple static SceneCapture2D in MultiMap to represent the spectator view but you could do a lot of things like move it with Sequencer or control it via another player. Lot of fun in perspective. See Input Map for toggle button.

FIXES: 

Motion controllers meshes not spawning if Guardian or Chaperone are not set [FIXED]
Now a default Guardian or Chaperone will be used if boundaries are not set correctly

Debug line is not set correctly in multiplayer [FIXED]

(VIVE) Left and Right Grip buttons on Vive controllers are not mapped correctly [FIXED]
Left and Right Grip buttons on each Vive controllers are now mapped at the same function

(VERSION 4.17 with OCULUS) Guardian boundaries and corners not detected correctly [FIXED]

NavModifiersVolume (NavAreaNull) have inconsistent behaviour since Unreal 4.16[TEMPORARY FIX]
For now I ditched the NavMesh and teleport on static mesh, deny teleportation with floor angle and CustomNavArea NavModifierVolume, until Navigation is more stable in Unreal.

Reworked a bit the input map for consistency between Rift/Vive and to accommodate the fact that since 4.16 the left and right grip buttons on the Vive controllers are no longer differentiated.

REPORTED BUGS / NOT BEEN ABLE TO REPRODUCE

“I've had the same teleportation direction issue through all of the versions of the template I've tested. For me the forward direction seems to be the "right->" direction of my playspace. Any luck reproducing the issue with teleport rotation on the vive? There are times it seems like I'm 90-180 degrees from where I'm supposed to end up and its really confusing sometimes. I haven't been able to figure it out myself »
I’ve tested the template with 4.16 and 4.17 version with a Vive; did not have any problem with playground orientation. Something to do with SteamVR setup?

“I'm still experiencing the laggy behaviour on the client side though. I wonder if I'm the only one? It's really apparent; for example when I'm just waving my hand around, the hand movement is quite glitchy”
As already reported, I’ve had vastly different experiences concerning lag, mainly with Steam: depending on the hour and the network load, lag is sometimes unnoticeable, sometimes heavy. Barely noticeable in LAN. Slowest through Oculus Network with Avatars.
I’ve had also few reports on lagginess increasing with time. Again, I did not notice that but I would suggest to play with Project Settings / Garbage Collection. From what I’ve tested I don’t have mem leaks but it surely can be improved through better garbage collection.
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
