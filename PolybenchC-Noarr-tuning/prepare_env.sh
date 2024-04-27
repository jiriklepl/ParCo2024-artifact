#!/bin/bash

export VENV_DIR=${VENV_DIR:-env}

if [ ! -d "$VENV_DIR" ]; then
    python3 -m venv "$VENV_DIR"
fi

source "$VENV_DIR/bin/activate"

pip install -r requirements.txt
