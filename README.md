# ChainCounter
Develop a chain counter for a boat. The chain counter should be fully integrated with the ship's navigation system and offer features such as

* Bright display at helm
* Display:
  *length of chain out,
  * % of total length out,
  * current scope (chainOut/depth),
  * gypsy status (idle, up, down, freefall, bound and fault condition)
* Remote control of windlass
* Automatic anchor watch activation and cancellation
* Alarms and warnings (end of chain, insufficient scope, bound gypsy)
* Parameters configurable via web page (gypsy circumference, chain length, alarm limits, anchor watch safety factor)
* Integration via open RESTful interface

The Chain counter should be engineered using Open Source software and hardware to keep costs down and allow it to be easily integrated with other on-board systems.

## Hardware Design
See the [hardware directory](./hardware) for further information. 
We are using [KiCad]([Ihttps://www.kicad.org/) for hardware design. See the [Schematic of current version](./hardware/KiCad/schematic_current_version.pdf).
