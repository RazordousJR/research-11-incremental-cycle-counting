#!/usr/bin/env bash
# Builds main.pdf for JGAA in a scratch copy, so no LaTeX aux file lands in the repository. Usage: bash docs/paper/jgaa/build.sh <scratch dir>
set -eu
here="$(cd "$(dirname "$0")" && pwd)"
out="${1:?scratch dir}"
rm -rf "$out" && mkdir -p "$out/paper/jgaa" "$out/figures"
cp -r "$here/." "$out/paper/jgaa/" && cp -r "$here/../../figures/pdf" "$out/figures/"
cd "$out/paper/jgaa"
pdflatex -interaction=nonstopmode main.tex > pass1.log
bibtex main > bibtex.log
pdflatex -interaction=nonstopmode main.tex > pass2.log
pdflatex -interaction=nonstopmode main.tex > pass3.log
grep -E "^!|Overfull|undefined" pass3.log || true
grep -i "^Warning" main.blg || true
cp main.pdf "$here/main.pdf"
