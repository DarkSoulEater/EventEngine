#!/usr/bin/bash

if [ "$EUID" -ne 0 ]; then
  echo "./install.sh must be run under root!"
  exit
fi

apt install -y texlive-latex-base
apt install -y texlive-fonts-recommended
apt install -y texlive-bibtex-extra
apt install -y texlive-fonts-extra
apt install -y texlive-latex-extra
apt install -y texlive-lang-cyrillic
apt install -y inkscape
apt install -y latexmk
