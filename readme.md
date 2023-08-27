# NoThrottle
## Abstract
The new Intel cpus with P- and E-cores have different performance profiles. Windows 11 might schedule the audio thread to an E-core when a the window is hidden, minimised or not visible to the user. This can be prevented by disabling throttling. This plugin takes care of that.

### Details
This plugin disables throttling for the audio thread. However, for the GUI thread throttling is enabled. To make sure this behavior cannot be overriden by other components, this is repeated every second.
