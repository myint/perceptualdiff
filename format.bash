#!/bin/bash

astyle \
    --add-brackets \
    --align-reference=name \
    --align-pointer=name \
    --indent=tab \
    --pad-header \
    --pad-oper \
    --unpad-paren \
    *.h \
    *.cpp
