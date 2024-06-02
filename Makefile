TARGET = boru
SRCDIR = source
INCDIR = include

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(SRCDIR)/%.o)

CC = gcc
LIBS = -lcrypt
CFLAGS = -Wall -Wextra -Werror -Wl,-z,now
CFLAGS_RELEASE = $(CFLAGS) -O2 -s -D_FORTIFY_SOURCE=2
CFLAGS_DEBUG = $(CFLAGS) -O0 -g -fsanitize=undefined

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -I$(INCDIR) $(OBJECTS) -o $(TARGET) $(LIBS)

all: release

release: CFLAGS := $(CFLAGS_RELEASE)
release: $(TARGET)

debug: CFLAGS := $(CFLAGS_DEBUG)
debug: $(TARGET)

install: $(TARGET)
	install -m 4755 $(TARGET) /usr/bin/$(TARGET)

uninstall:
	rm -rf /usr/bin/$(TARGET)
	rm -rf /etc/boru.conf

clean:
	rm -rf $(SRCDIR)/*.o $(TARGET)

.PHONY: all debug release install uninstall clean