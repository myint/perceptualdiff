#!/bin/bash

astyle \
    --add-brackets \
    --align-reference=name \
    --align-pointer=name \
    --indent=tab \
    *.h \
    *.cpp
