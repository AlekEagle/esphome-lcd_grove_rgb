#pragma once

#include "esphome/core/component.h"
#include "esphome/core/time.h"
#include "esphome/components/i2c/i2c.h"
// We are not using the LCD base class because it doesn't work with the Grove RGB LCD display.

#include <map>
#include <vector>

namespace esphome
{
  namespace lcd_grove_rgb
  {

    class LCDGroveRGB : public PollingComponent, public i2c::I2CDevice
    {
    public:
      void set_backlight_address(uint8_t address) { this->backlight_address_ = address; }
      void set_dimensions(uint8_t columns, uint8_t rows)
      {
        this->columns_ = columns;
        this->rows_ = rows;
      }
      void set_user_defined_char(uint8_t pos, const std::vector<uint8_t> &data) { this->user_defined_chars_[pos] = data; }
      void set_writer(std::function<void(LCDGroveRGB &)> &&writer) { this->writer_ = std::move(writer); }
      void setup() override;
      float get_setup_priority() const override;
      void update() override;

      // Display commands
      void clear();
      void display();
      void no_display();

      void set_cursor(uint8_t column, uint8_t row);

      // Backlight commands
      void backlight();
      void backlight(uint8_t brightness);
      void backlight(uint8_t red, uint8_t green, uint8_t blue);
      void no_backlight();
      // TODO: Add the other display commands.

      // Print commands
      /// Print the given text at the specified column and row.
      void print(uint8_t column, uint8_t row, const char *str);
      /// Print the given string at the specified column and row.
      void print(uint8_t column, uint8_t row, const std::string &str);
      /// Print the given text at column=0 and row=0.
      void print(const char *str);
      /// Print the given string at column=0 and row=0.
      void print(const std::string &str);
      /// Evaluate the printf-format and print the text at the specified column and row.
      void printf(uint8_t column, uint8_t row, const char *format, ...) __attribute__((format(printf, 4, 5)));
      /// Evaluate the printf-format and print the text at column=0 and row=0.
      void printf(const char *format, ...) __attribute__((format(printf, 2, 3)));

      /// Evaluate the strftime-format and print the text at the specified column and row.
      void strftime(uint8_t column, uint8_t row, const char *format, ESPTime time) __attribute__((format(strftime, 4, 0)));
      /// Evaluate the strftime-format and print the text at column=0 and row=0.
      void strftime(const char *format, ESPTime time) __attribute__((format(strftime, 2, 0)));

      /// Load custom char to given location
      void loadchar(uint8_t location, uint8_t charmap[]);

    protected:
      bool is_four_bit_mode() { return true; };
      bool is_5x10_dots() { return false; };
      void i2c_send_byte(const uint8_t *value);
      void i2c_send_byteS(uint8_t *value, uint8_t len);
      void i2c_send_byte_backlight(const uint8_t *value);
      void i2c_send_byteS_backlight(uint8_t *value, uint8_t len);

      void command_(uint8_t value);
      void send_(uint8_t value);
      void send_backlight_(uint8_t reg, uint8_t value);
      virtual void call_writer() { this->writer_(*this); };

      i2c::I2CDevice *backlight_;
      uint8_t backlight_address_{0x62};
      uint8_t columns_;
      uint8_t rows_;
      uint8_t current_column_;
      uint8_t display_function_;
      uint8_t display_control_;
      uint8_t display_mode_;
      std::function<void(LCDGroveRGB &)> writer_;
      std::map<uint8_t, std::vector<uint8_t>> user_defined_chars_;
    };

  } // namespace lcd_grove_rgb
} // namespace esphome