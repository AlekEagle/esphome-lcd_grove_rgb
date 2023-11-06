#include "lcd_grove_rgb.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/hal.h"

namespace esphome
{
  namespace lcd_grove_rgb
  {

    static const char *const TAG = "lcd_grove_rgb";

    // First set bit determines command, bits after that are the data.
    static const uint8_t LCD_DISPLAY_COMMAND_CLEAR_DISPLAY = 0x01;
    static const uint8_t LCD_DISPLAY_COMMAND_RETURN_HOME = 0x02;
    static const uint8_t LCD_DISPLAY_COMMAND_ENTRY_MODE_SET = 0x04;
    static const uint8_t LCD_DISPLAY_COMMAND_DISPLAY_CONTROL = 0x08;
    static const uint8_t LCD_DISPLAY_COMMAND_CURSOR_SHIFT = 0x10;
    static const uint8_t LCD_DISPLAY_COMMAND_FUNCTION_SET = 0x20;
    static const uint8_t LCD_DISPLAY_COMMAND_SET_CGRAM_ADDR = 0x40;
    static const uint8_t LCD_DISPLAY_COMMAND_SET_DDRAM_ADDR = 0x80;

    static const uint8_t LCD_DISPLAY_ENTRY_SHIFT_INCREMENT = 0x01;
    static const uint8_t LCD_DISPLAY_ENTRY_LEFT = 0x02;

    static const uint8_t LCD_DISPLAY_DISPLAY_BLINK_ON = 0x01;
    static const uint8_t LCD_DISPLAY_DISPLAY_CURSOR_ON = 0x02;
    static const uint8_t LCD_DISPLAY_DISPLAY_ON = 0x04;

    static const uint8_t LCD_DISPLAY_FUNCTION_2_LINE = 0x08;

    static const uint8_t LCD_BACKLIGHT_ADDRESS_V5 = 0x30;
    static const uint8_t LCD_BACKLIGHT_REG_MODE1 = 0x00;
    static const uint8_t LCD_BACKLIGHT_REG_MODE2 = 0x01;
    static const uint8_t LCD_BACKLIGHT_REG_OUTPUT = 0x08;

    void LCDGroveRGB::i2c_send_byte(const uint8_t *value) { this->write(value, 1); }

    void LCDGroveRGB::i2c_send_byteS(uint8_t *value, uint8_t len) { this->write(value, len); }

    void LCDGroveRGB::i2c_send_byte_backlight(const uint8_t *value) { this->backlight_->write(value, 1); }

    void LCDGroveRGB::i2c_send_byteS_backlight(uint8_t *value, uint8_t len) { this->backlight_->write(value, len); }

    void LCDGroveRGB::setup()
    {
      ESP_LOGCONFIG(TAG, "Setting up Grove RGB LCD Display...");

      this->display_function_ |= LCD_DISPLAY_FUNCTION_2_LINE;

      // Commands can only be sent 40ms after boot-up, so let's wait if we're close
      delay(50);

      // Send the function set command sequence four times. (This is not a mistake, it's how the Grove RGB LCD display
      // is initialized in the first-party Arduino library, so we mimic that behavior.)
      this->command_(LCD_DISPLAY_COMMAND_FUNCTION_SET | this->display_function_); // First transmission
      ESP_LOGVV(TAG, "Function set command sent. (First transmission)");
      // Wait for more than 4.1ms
      delayMicroseconds(4500);
      this->command_(LCD_DISPLAY_COMMAND_FUNCTION_SET | this->display_function_); // Second transmission
      ESP_LOGVV(TAG, "Function set command sent. (Second transmission)");
      // Wait for more than 100us
      delayMicroseconds(150);
      this->command_(LCD_DISPLAY_COMMAND_FUNCTION_SET | this->display_function_); // Third transmission
      ESP_LOGVV(TAG, "Function set command sent. (Third transmission)");
      // Wait for more than 100us
      delayMicroseconds(150);
      this->command_(LCD_DISPLAY_COMMAND_FUNCTION_SET | this->display_function_); // Fourth transmission
      ESP_LOGVV(TAG, "Function set command sent. (Fourth transmission)");

      // Turn on the display with no cursor or blinking as the default
      this->display();

      // Clear the display
      this->clear();

      // Initialize to default text direction (for romance languages)
      this->display_mode_ = LCD_DISPLAY_ENTRY_LEFT;
      this->command_(LCD_DISPLAY_COMMAND_ENTRY_MODE_SET | this->display_mode_);

      // Initialize the backlight
      this->backlight_ = new i2c::I2CDevice();
      this->backlight_->set_i2c_address(this->backlight_address_);
      this->backlight_->set_i2c_bus(this->bus_);

      if (this->backlight_address_ == LCD_BACKLIGHT_ADDRESS_V5)
      {
        this->send_backlight_(0x00, 0x07);
        delayMicroseconds(200);
        this->send_backlight_(0x04, 0x15);
      }
      else
      {
        this->send_backlight_(LCD_BACKLIGHT_REG_MODE1, 0x00);
        this->send_backlight_(LCD_BACKLIGHT_REG_OUTPUT, 0xFF);
        this->send_backlight_(LCD_BACKLIGHT_REG_MODE2, 0x20);
      }

      this->backlight();
    }

    float LCDGroveRGB::get_setup_priority() const { return setup_priority::PROCESSOR; }

    void LCDGroveRGB::update()
    {
      if (this->clear_on_update_)
        this->clear();
      if (this->home_on_update_)
        this->home();
      this->call_writer();
    }

    void LCDGroveRGB::clear()
    {
      this->command_(LCD_DISPLAY_COMMAND_CLEAR_DISPLAY);
      ESP_LOGVV(TAG, "Clear display command sent.");
      delayMicroseconds(2000);
    }

    void LCDGroveRGB::home()
    {
      this->command_(LCD_DISPLAY_COMMAND_RETURN_HOME);
      ESP_LOGVV(TAG, "Return home command sent.");
      delayMicroseconds(2000);
    }

    void LCDGroveRGB::display()
    {
      this->display_control_ |= LCD_DISPLAY_DISPLAY_ON;
      this->command_(LCD_DISPLAY_COMMAND_DISPLAY_CONTROL | this->display_control_);
      ESP_LOGVV(TAG, "Display on command sent.");
    }

    void LCDGroveRGB::no_display()
    {
      this->display_control_ &= ~LCD_DISPLAY_DISPLAY_ON;
      this->command_(LCD_DISPLAY_COMMAND_DISPLAY_CONTROL | this->display_control_);
      ESP_LOGVV(TAG, "Display off command sent.");
    }

    void LCDGroveRGB::set_cursor(uint8_t column, uint8_t row)
    {
      column = (row == 0 ? column | 0x80 : column | 0xc0);
      this->command_(column);
      ESP_LOGVV(TAG, "Set cursor command sent.");
    }

    void LCDGroveRGB::backlight() { this->backlight(255); }

    void LCDGroveRGB::backlight(uint8_t brightness) { this->backlight(brightness, brightness, brightness); }

    void LCDGroveRGB::backlight(uint8_t red, uint8_t green, uint8_t blue)
    {
      if (this->backlight_address_ == LCD_BACKLIGHT_ADDRESS_V5)
      {
        this->send_backlight_(0x06, red);
        ESP_LOGVV(TAG, "Backlight command sent. (Red)");
        this->send_backlight_(0x07, green);
        ESP_LOGVV(TAG, "Backlight command sent. (Green)");
        this->send_backlight_(0x08, blue);
        ESP_LOGVV(TAG, "Backlight command sent. (Blue)");
      }
      else
      {
        this->send_backlight_(0x04, red);
        ESP_LOGVV(TAG, "Backlight command sent. (Red)");
        this->send_backlight_(0x03, green);
        ESP_LOGVV(TAG, "Backlight command sent. (Green)");
        this->send_backlight_(0x02, blue);
        ESP_LOGVV(TAG, "Backlight command sent. (Blue)");
      }
    }

    void LCDGroveRGB::no_backlight() { this->backlight(0, 0, 0); }

    void LCDGroveRGB::print(uint8_t column, uint8_t row, const char *str)
    {
      this->set_cursor(column, row);
      this->print(str);
    }

    void LCDGroveRGB::print(uint8_t column, uint8_t row, const std::string &str)
    {
      this->set_cursor(column, row);
      this->print(str);
    }

    void LCDGroveRGB::print(const char *str)
    {
      for (uint8_t i = 0; str[i] != '\0'; i++)
      {
        this->send_(str[i]);
      }
      ESP_LOGVV(TAG, "Print command sent.");
    }

    void LCDGroveRGB::print(const std::string &str)
    {
      for (uint8_t i = 0; i < str.length(); i++)
      {
        this->send_(str[i]);
      }
    }

    void LCDGroveRGB::printf(uint8_t column, uint8_t row, const char *format, ...)
    {
      va_list args;
      va_start(args, format);
      char buffer[256];
      int ret = vsnprintf(buffer, sizeof(buffer), format, args);
      va_end(args);
      if (ret > 0)
      {
        this->set_cursor(column, row);
        this->print(buffer);
      }
    }

    void LCDGroveRGB::printf(const char *format, ...)
    {
      va_list args;
      va_start(args, format);
      char buffer[256];
      int ret = vsnprintf(buffer, sizeof(buffer), format, args);
      va_end(args);
      if (ret > 0)
      {
        this->print(buffer);
      }
      va_end(args);
    }

    void LCDGroveRGB::strftime(uint8_t column, uint8_t row, const char *format, ESPTime time)
    {
      this->set_cursor(column, row);
      this->strftime(format, time);
    }

    void LCDGroveRGB::strftime(const char *format, ESPTime time)
    {
      char buffer[256];
      int ret = time.strftime(buffer, sizeof(buffer), format);
      if (ret > 0)
        this->print(buffer);
    }

    void LCDGroveRGB::loadchar(uint8_t location, uint8_t charmap[])
    {
      location &= 0x7; // we only have 8 locations 0-7
      this->command_(LCD_DISPLAY_COMMAND_SET_CGRAM_ADDR | (location << 3));
      for (uint8_t i = 0; i < 8; i++)
      {
        this->send_(charmap[i]);
      }
    }

    void LCDGroveRGB::command_(uint8_t value)
    {
      uint8_t data[2] = {0x80, value};
      this->i2c_send_byteS(data, 2);
    }

    void LCDGroveRGB::send_(uint8_t value)
    {
      uint8_t data[2] = {0x40, value};
      this->i2c_send_byteS(data, 2);
    }

    void LCDGroveRGB::send_backlight_(uint8_t reg, uint8_t value)
    {
      uint8_t data[2] = {reg, value};
      this->i2c_send_byteS_backlight(data, 2);
    }

  } // namespace lcd_grove_rgb
} // namespace esphome