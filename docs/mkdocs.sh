#!/usr/bin/bash

cd ..
pandoc README.md --pdf-engine=lualatex -o docs/f1801q128-dct.pdf
cd docs
