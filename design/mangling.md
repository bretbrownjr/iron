# Iron Name Mangling #

## Background ##

Iron will include a namespace/module feature, though the nomenclature and
detailed design of this feature is still in development. This file is intended
to capture thoughts about that development until a detailed design document can
be developed.

## Justification ##

Every sufficiently complex program needs the ability to differentiate between
similar parts of a program. Take, for example, printing character strings to
the standard output stream.

