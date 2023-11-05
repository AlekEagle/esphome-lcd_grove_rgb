# ESPHome LCD Grove RGB

This is a simple component to use the Grove-LCD RGB backlight display with ESPHome.

## Features

- Display text on the LCD
- Set the backlight color
- Set the backlight brightness (white only)

## Usage

```yaml
# Example configuration entry
display:
  - platform: lcd_grove_rgb
    dimensions: 16x2 # optional, default: 16x2
    address: 0x3e # optional, default: 0x3e
    backlight: 0x62 # optional, default: 0x62
    lambda: |-
      it.print(0, 0, "Hello World!");
    update_interval: 1s
```

## Lambda Methods

### print

Uses the same print and printf methods as other LCD display components. See the [LCD Display Component](https://esphome.io/components/display/lcd_display) for more information.

### backlight

Set the backlight color. The color is an RGB triplet, where each value is between 0 and 255. The default color is white (255, 255, 255).

```cpp
it.backlight(); // Turn on the backlight with the default color (white)

it.backlight(128); // Set the backlight brightness to 128 (white)

it.backlight(255, 0, 0); // Set the backlight color to red

it.no_backlight(); // Turn off the backlight
```

### clear

Clears the display.

### home

Sets the cursor to the home position.

### set_cursor

Sets the cursor to the specified position.

```cpp
it.set_cursor(0, 0); // Set the cursor to the first column of the first row
```

### display

Turns the display on.

### no_display

Turns the display off.
