TARGET = boru
SRCDIR = source
INCDIR = include
BUILDDIR = build

SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))

CC = gcc
LIBS = -lcrypt
CFLAGS = -Wall -Wextra -Werror -Wl,-z,now
CFLAGS_RELEASE = $(CFLAGS) -O2 -s -D_FORTIFY_SOURCE=2
CFLAGS_DEBUG = $(CFLAGS) -O0 -g -fsanitize=undefined

all: release

release: CFLAGS := $(CFLAGS_RELEASE)
release: $(BUILDDIR) $(TARGET)

debug: CFLAGS := $(CFLAGS_DEBUG)
debug: $(BUILDDIR) $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -I$(INCDIR) $(OBJECTS) -o $(BUILDDIR)/$(TARGET) $(LIBS)

install: $(TARGET)
	install -m 4755 $(BUILDDIR)/$(TARGET) /usr/bin/$(TARGET)

uninstall:
	rm -rf /usr/bin/$(TARGET)
	rm -rf /etc/boru.conf

clean:
	rm -rf $(BUILDDIR)

.PHONY: all debug release install uninstall clean