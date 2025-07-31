struct UID
{
    uint8_t uid[7]; // UID can be up to 8 bytes long
    uint8_t length; // Length of the UID
};

// It contains the payload of the NFC Chip, which is 16 bytes long
struct COMMAND
{
    uint8_t funcion;    // Function code for the command
    uint32_t timeValue; // Data bytes for the command
    uint8_t red;        // Red value for the command
    uint8_t green;      // Green value for the command
    uint8_t blue;       // Blue value for the command
    uint8_t checksum;   // Checksum for the command
};

void nfc_setup();
void nfc_loop();
COMMAND cpy();
void exec(COMMAND cmd);
bool kill_timer();
void load_timer();