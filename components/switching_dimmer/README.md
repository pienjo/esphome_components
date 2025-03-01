# Switching Dimmer

This component allows lights with a built-in dimmer (controlled by power toggles) to be controlled using a smart relay switch (e.g. a Sonoff S20).

These lights typically turn on at full brightness and can be cycled between discrete dimming levels by briefly interrupting power. See https://products.trio-lighting.com/474410179-2/ for an example.

Example:
```yaml
light:
  - platform: monochromatic
    name: "Texel"
    gamma_correct: 1.0
    default_transition_length: 0s
    output: toggle_output

output:
  - platform: switching_dimmer
    id: toggle_output
    pin: GPIO12
    nr_phases: 3    # There are levels of intensity (besides off)
    direction: down # possible values: [ up, down ]
```

A direction of ```down``` means that the dimmer switches to a *lower* state every time the power is blipped (e.g. 100% -> 66% -> 33% -> 100%).

A direction of ```up``` means that the dimmer switches to a *higher* state every time the power is blipped (e.g. 33% -> 66% -> 100% -> 33%)
