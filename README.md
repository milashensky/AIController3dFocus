# BaseGameUtils

a plugin for Unreal Engine 5.6 with a bunch of classes and utilities. can't be bothered to do the same scaffolding for every project :)

provides:

* `AIController3dFocus`: a controller which uses pitch value for `setFocalPoint`, and not dropping it as a default one
* `SensingCharacter`: a character with ability to define eyes view point socket by setting `Sense Socket Bone Name` in blueprints. to help with aiming, also has a flag `UseSenseSocket`. if troubles with an animation, disable it once `SetFocus` is called.
* `ClearAllTimers` util: clears all timers for the object
* `DestroyAIController` util: allows to destroy ai controller. by default you not suppose to destroy the controller, but if you ain't planning on resurrecting the pawn after death, I don't see why not. unpossesses the pawn, clears timers, destroys the controller.

### Installation

* copy the plugin to the Engine's Plugins/Marketplace folder (`Path/to/Engine/Plugins/Marketplace`)
* add plugin to the project
* ...
* profit
