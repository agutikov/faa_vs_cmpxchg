#!/usr/bin/env bash
# Create a local virtualenv at ./venv and install plot2d.py's Python dependencies.
set -euo pipefail

VENV_DIR="${VENV_DIR:-./.venv}"

if [ ! -d "$VENV_DIR" ]; then
    python3 -m venv "$VENV_DIR"
fi

# shellcheck disable=SC1090
source "$VENV_DIR/bin/activate"

pip install --upgrade pip
pip install docopt pandas matplotlib numpy

echo
echo "venv ready at $VENV_DIR — activate with: source $VENV_DIR/bin/activate"
