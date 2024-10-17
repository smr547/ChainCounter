# ChainCounter
Develop a chain counter for a boat. The chain counter should be fully integrated with the ship's navigation system and offer features such as

* Bright display at helm
* Display:
  * length of chain out,
  * % of total length out,
  * current scope (chainOut/depth),
  * gypsy status (idle, up, down, freefall, bound and fault condition)
* Anchor location: Home, dangling, dropping, lifting, free falling, on bottom, set
* Anchor last known position (latitude, longitude and depth -- in case the anchor and chain are lost)
* Automatic anchor watch activation and cancellation
* Notifications
** Deploying anchor
** Anchor on bottom
** Anchor holding
** Lifting anchor
** Anchor weighed
** Anchor homed
* Voice notification and warnings
* Alarms and warnings:
** end of chain,
** insufficient scope,
** bound gypsy,
** anchor dragging,
** boat left anchor zone
* All information available remotely (web page and mobile app)
* Remote control of windlass (up / down buttons)
* Parameters configurable via web page (gypsy circumference, chain length, alarm limits, anchor watch safety factor)
* Integration via open RESTful interface

The Chain counter should be engineered using Open Source software and hardware to keep costs down and allow it to be easily integrated with other on-board systems.

## Parameters reported

* ``navigation.anchor.chainOut`` -- length of chain out
* ``navigation.anchor.chainPercent`` -- chain out as a percentage of chain available
* ``navigation.anchor.gypsyStatus`` -- idle, up, down, freefall, bound, fault(code)
* ``navigation.anchor.deploymentStatus`` -- home, hanging, inWater, onBottom or unknown (depth data unavailable)
* ``navigation.anchor.bottomPosition`` -- lat/long of anchor on bottom (if depth and GPS data unavailable)
* ``navigation.anchor.scopeOut`` -- chain out as a factor of depth (if depth data unavailable)
* ``navigation.anchor.gypsyCircumference`` -- circumference of gypsy *
* ``navigation.anchor.chainLength`` -- length of chain installed *

(* indicates parameter configurable via web page or RESTful interface)

## Events reported

* chain stopped / moving
* chain on bottom
* chain weighed (off bottom)
* chain home


## Hardware Design
See the [hardware directory](./hardware) for further information. 
We are using [KiCad]([Ihttps://www.kicad.org/) for hardware design. See the [Schematic of current version](./hardware/KiCad/schematic_current_version.pdf).
