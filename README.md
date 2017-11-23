This repository contains source code of the articles which are hosted at [`http://gudok.xyz/`](http://gudok.xyz/)


### Required Debian packges

  * dia             -- to edit diagrams
  * libreoffice     -- to edit charts
  * asciidoctor     -- to build article
  * pygments        -- to build article (source code highlighting)
  * texlive         -- to build article (math rendering)
  * dvipng          -- to build article (math rendering)
  * sassc           -- to compile "lepota" stylesheet from SASS template
  * yui-compressor  -- to compress "lepota" stylesheet
 

### Build

  * ./build.sh        -- build all modules
  * ./build.sh clean  -- clean all modules


### Dia hints [0.97.3]

  1. Sometimes (but not always) nearly straight arc is saved as straight line.
     If this has happened, then you need to create an empty diagram and copy-paste everything except the problematic arc.
     Do not copy the problematic arc into the new file -- it seems that it "spoils" diagram permamently.
     (I suspect something like that math is done in ratios, and this nearly straight arc makes global denominator very huge).
     After copypasting, you may create new arc from scratch or copy similar arc from another dia file that doesn't suffer 
     from this problem.

  2. Sometimes, when thick border of some object is lying on the very edge of diagram, exporting diagram to svg
     makes this border thin.
     It is weird to see, for example, rectangle with three expected 5px borders and one unexpected very thin border (1px).
     This usually happens with large diagrams.
     Solution is to create invisible, all-image large, transparent rectangle with white border and use it as dummy substrate.


### LibreOffice hints [4.3.3.2]

  1. When top wall of chart coincides with horizontal grid line (always unless Y limit is manually overriden),
     it is rendered with grid line style, not with a wall line style.
     For example, if grid line is 10% gray and wall line is 50% gray, then left, right and bottom wall lines are 50% (expected),
     but top wall line is only 10% (unexpected).
     Solution is to add third coincider -- X2 axis.
     Because it has even higher priority than above two, you may now set its style to be the same as for wall line.
     You also need to get rid of axis text by reducing its font size to 1 and making it white.

  2. If you want to create two Y axes for a single data series (Y1 for absolute values and Y2 for percent of max),
     then you have to create two data series in two different columns, attach each of them to corresponding Y axis
     and manually adjust min/max values for Y1/Y2 so that two data series align one atop of another one invisibly.


### Math hints

  1. Comma right after formula may be wrapped to the next line by browser.
     To avoid that, include comma as part of the formula itself.

