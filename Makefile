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

river-bed-theme:
	mkdir -p res/panels-river-bed
	cp res/panels/* res/panels-river-bed/
	sed -i 's/#e6e6e6/#4c5b5c/g' res/panels-river-bed/* # Background
	sed -i 's/#f9f9f9/#596868/g' res/panels-river-bed/* # Input area
	sed -i 's/#ccc/#414f4f/g' res/panels-river-bed/* # Output area
	sed -i 's/#848484/#1f2323/g' res/panels-river-bed/* # Section header
	sed -i 's/#606060/#1f2323/g' res/panels-river-bed/* # Text on ouput area
	sed -i 's/#010000/#b3b3b3/g' res/panels-river-bed/* # Black text
	#sed -i 's/#f8f9f9/#b3b3b3/g' res/panels-river-bed/* # White text

shark-theme:
	mkdir -p res/panels-shark
	cp res/panels/* res/panels-shark/
	sed -i 's/#e6e6e6/#262730/g' res/panels-shark/* # Background
	sed -i 's/#f9f9f9/#3a3e49/g' res/panels-shark/* # Input area
	sed -i 's/#ccc/#18191e/g' res/panels-shark/* # Output area
	sed -i 's/#848484/#c8cace/g' res/panels-shark/* # Section header
	sed -i 's/#606060/#c8cace/g' res/panels-shark/* # Text on ouput area
	sed -i 's/#010000/#c8cace/g' res/panels-shark/* # Black text
	#sed -i 's/#f8f9f9/#b3b3b3/g' res/panels-shark/* # White text


oxford-blue-theme:
	mkdir -p res/panels-oxford-blue
	cp res/panels/* res/panels-oxford-blue/
	sed -i 's/#e6e6e6/#364156/g' res/panels-oxford-blue/* # Background
	sed -i 's/#f9f9f9/#45546b/g' res/panels-oxford-blue/* # Input area
	sed -i 's/#ccc/#2c3644/g' res/panels-oxford-blue/* # Output area
	sed -i 's/#848484/#020405/g' res/panels-oxford-blue/* # Section header
	sed -i 's/#606060/#020405/g' res/panels-oxford-blue/* # Text on ouput area
	sed -i 's/#010000/#b6bac1/g' res/panels-oxford-blue/* # Black text
	#sed -i 's/#f8f9f9/#b3b3b3/g' res/panels-oxford-blue/* # White text

cod-gray-theme:
	mkdir -p res/panels-cod-gray
	cp res/panels/* res/panels-cod-gray/
	sed -i 's/#e6e6e6/#1b1b1b/g' res/panels-cod-gray/* # Background
	sed -i 's/#f9f9f9/#333333/g' res/panels-cod-gray/* # Input area
	sed -i 's/#ccc/#0a0a0a/g' res/panels-cod-gray/* # Output area
	sed -i 's/#848484/#999999/g' res/panels-cod-gray/* # Section header
	sed -i 's/#606060/#999999/g' res/panels-cod-gray/* # Text on ouput area
	sed -i 's/#010000/#afafaf/g' res/panels-cod-gray/* # Black text
	#sed -i 's/#f8f9f9/#b3b3b3/g' res/panels-cod-gray/* # White text

firefly-theme:
	mkdir -p res/panels-firefly
	cp res/panels/* res/panels-firefly/
	sed -i 's/#e6e6e6/#0c2d32/g' res/panels-firefly/* # Background
	sed -i 's/#f9f9f9/#2c5256/g' res/panels-firefly/* # Input area
	sed -i 's/#ccc/#061c1e/g' res/panels-firefly/* # Output area
	sed -i 's/#848484/#b6bfbf/g' res/panels-firefly/* # Section header
	sed -i 's/#606060/#b6bfbf/g' res/panels-firefly/* # Text on ouput area
	sed -i 's/#010000/#b6bfbf/g' res/panels-firefly/* # Black text
	#sed -i 's/#f8f9f9/#b3b3b3/g' res/panels-firefly/* # White text

allthemes: river-bed-theme shark-theme oxford-blue-theme cod-gray-theme firefly-theme

updatethemes: allthemes install


# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk
