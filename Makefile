CC = gcc
CFLAGS = -Wall -Wextra -std=c11 `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0` -lm
TARGET = image_compressor
SOURCES = main.c gui.c image_processor.c sparse_matrix.c
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

install-deps:
	@echo "Installing dependencies..."
	@if command -v brew >/dev/null 2>&1; then \
		brew install gtk+3 || echo "GTK+3 already installed or install manually"; \
	else \
		echo "Please install GTK+3 development libraries:"; \
		echo "  macOS: brew install gtk+3"; \
		echo "  Ubuntu/Debian: sudo apt-get install libgtk-3-dev"; \
		echo "  Fedora: sudo dnf install gtk3-devel"; \
	fi

run: $(TARGET)
	./$(TARGET)

