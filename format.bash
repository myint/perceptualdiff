#!/bin/bash

astyle \
    --add-brackets \
    --align-pointer=name \
    --align-reference=name \
    --indent=tab \
    --pad-header \
    --unpad-paren \
    *.cpp \
    *.h
