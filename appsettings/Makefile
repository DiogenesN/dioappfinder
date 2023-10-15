CC = gcc
CFLAGS = -Wall -flto -Wextra -Wpedantic -march=native -funroll-loops -export-dynamic -fomit-frame-pointer
LDFLAGS += $(shell pkg-config --libs gtk4 glib-2.0)
CFLAGS += $(shell pkg-config --cflags gtk4 glib-2.0)
SRCFILES = main-settings.c
OBJFILES = $(patsubst src/%.c, %.o, $(SRCFILES))
TARGET = dioappfinder-settings
CFLAGS += -Iappsettings/
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "\nSuccessfully built $(TARGET)!\nType 'make run' to test or 'sudo make install' to install."

%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: run

run: $(TARGET)
	./$(TARGET)

install: $(TARGET)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)

clean:
	rm -f $(OBJFILES) $(TARGET)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
