# If RACK_DIR is not defined when calling the Makefile, default to two directories above
RACK_DIR ?= ../..

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS +=
CXXFLAGS +=

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine, but they should be added to this plugin's build system.
LDFLAGS +=

# Add .cpp files to the build
SOURCES += $(wildcard src/*.cpp src/utility/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin and "plugin.json" are automatically added.
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)

teal-theme:
	mkdir -p res/panels-teal
	cp res/panels/* res/panels-teal/
	sed -i 's/#e6e6e6/#4c5b5c/g' res/panels-teal/* # Background
	sed -i 's/#f9f9f9/#566666/g' res/panels-teal/* # Input area
	sed -i 's/#ccc/#414f4f/g' res/panels-teal/* # Output area
	sed -i 's/#848484/#414f4f/g' res/panels-teal/* # Section header
	#sed -i 's/#010000/#b3b3b3/g' res/panels-teal/* # Black text
	#sed -i 's/#f8f9f9/#b3b3b3/g' res/panels-purple/* # White text

purple-theme:
	mkdir -p res/panels-purple
	cp res/panels/* res/panels-purple/
	sed -i 's/#e6e6e6/#262730/g' res/panels-purple/* # Background
	sed -i 's/#f9f9f9/#2e3038/g' res/panels-purple/* # Input area
	sed -i 's/#ccc/#18191e/g' res/panels-purple/* # Output area
	sed -i 's/#848484/#18191e/g' res/panels-teal/* # Section header
	sed -i 's/#010000/#b3b3b3/g' res/panels-purple/* # Black text
	#sed -i 's/#f8f9f9/#b3b3b3/g' res/panels-purple/* # White text


allthemes: teal-theme purple-theme


# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk
