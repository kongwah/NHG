#!/bin/csh -f

set cc = 1
while ( $cc <= 1875 )
   echo $cc
   qqq $cc
   @ cc = $cc + 1
end
