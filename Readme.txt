Autostart PRX
--------------
Hey, this plugin allows you to launch your games and applications on PSP startup by simply holding an assigned button. You can also hold the R Trigger and a configured button anywhere in the XMB to launch your assigned game. By using the included configurator you can bind your homebrew/ISO/PSX games to up to 10 different buttons and 1 to boot automatically(good for a cintro). Full instructions for use are below.

If anyone has any problems or suggestions you can email me at the address below and I'll take a look.

Hope you enjoy and thanks for downloading,
roe-ur-boat

Installation
-------------
1. Copy the PSP folder to the root of your memory stick.
2. If you have no other plugins, copy the seplugins folder to the root of your memory stick.
	However, if you have other plugins and/or you have used earlier revisions of this plugin you must add the line "ms0:/seplugins/AutoStart.prx"(without the quotes) to vsh.txt and delete any previous Config.ROE you may have.
3. Enable AutoStart.prx in recovery mode
4. Run the configurator from the XMB to configure the plugin.

ChangeLog
----------
Version 5.5
	Changed the hooking method, apps like irshell's exit game works correctly now.
	Fixed the bug where you couldn't turn back on the screen if turned off.
	The constant LED blinking on startup shouldn't happen anymore.
	Made the text in the options menu easier to understand.
	
Version 5.4:
	Now you can launch one of the assigned games anywhere in the XMB by holding the R Trigger and the corresponding button.
	Put some instructions on using the plugin in the readme, sorry about that.

Version 5.3:
	Fixed a bug I added when I added support for Bubbletunes Game Categories.
	Fixed a bug where the configurator would crash if an ISO had no name...
	New GUI courtesy of aspheric.

Version 5.2:
	Fixed a stack overflow bug that caused random crashes.
	
Version 5.1:
	Fixed a bug where the configurator would crash if an eboot had no name.
	Now scans GAME5XX for games too.
	Now supports GEN firmwares.
	Added support for Bubbletune's Game Categories plugin.
	
Version 5:
	Recoded both the Configurator and the plugin. Much more stable than previous releases.
	Configurator has both a new design and it is much easier to find your games.
	Got a new GUI design. (Thanks Richie and Jazzer)

Version 4:
	You can now bypass the autoboot by putting the WLAN switch up.
	You can launch PSX games now too.
	Rewrote the configurator and the configuration file parser. Configurator looks better and is more user friendly. (Thanks to Mc Cabe for the background)
	PRX size decreased thanks to recoding and psp-packer.
	No need to enable the prx for game.txt now.

Version 3:
	Now you can assign a homebrew to autoboot. Handy for cintro. (No looping!)
	Now you can not only assign homebrew but backed up games too.

Version 2:
	Added more buttons
	Improved the configurator
	Fixed a bug that showed the SCE logo when you exited from a homebrew
	Descreased the prx size from 30kb to 9kb

Version 1.1:
	Descreased the prx size from 90kb to 30kb

Version 1:
	Initial release is initial.


Instructions for use
---------------------
Configurator:
- Main screen:
	At the main screen you can choose what button you want to configure or what to autoboot. Controls are shown on screen.

- Browser:
	This shows a list of all the games in your game folders and ISO folder for selection. Press X to select something or select to go back to the main screen.

- Options:
	Here, you can choose whether to enable the autoboot function and the wlan switch check. The wlan switch check is for when you have autoboot enabled but you only want the autoboot to work if you have the wlan switch off.

Plugin:
	To use the plugin, simply hold the button you configured your games to whilst turning on your PSP and it will launch. Or, if you want to launch something in the XMB you can hold the R Trigger and a configured button.
	
Credits
--------
aspheric for the current GUI design.
Coldbird for the new hooking api.
Lampworker for testing this release.
Everyone else who tested previous releases.

Contact
-------
You can contact me at my email address roe-ur-boat@hotmail.com
My twitter page http://twitter.com/roe_ur_boat
