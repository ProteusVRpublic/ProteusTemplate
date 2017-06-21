### Proteus Template###

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

# Latest Versions #

# Version 3.0 (June 21, 2017) #

# Latest changelog #
* Update to Unreal 4.16.1
* Everything has been redone from the ground. Rather than adapting Epic’s VR template, all functions have been redone with multiplayer in mind. Always lost my mind doing it but it’s done.
* VR Tracker: The mesh has been changed for a very simple one, preventing cooking problems. Adding/removing Vive Trackers is now working well whether the owner is a listen-server or a client.
* The grabbing functions and interactive objects are now fully compatible with the VR Interaction Kit (included).
* Major multiplayer optimization has been done, with very good replication on fast connections.
* All bugs related from previous version solved.
* Works Single or Multiplayer
* Oculus Avatar: pre-wiring done, integrate with my mini-plugin “Proteus Avatars” if present (soon)
* Inclusion of a 3rd person “Test Pawn”, for debugging and do all kind of tests


Please read the pdf document for the complete instructions. It is found at https://1drv.ms/f/s!Av77lIIxt2OY0XGGW8UDwykohjuT  