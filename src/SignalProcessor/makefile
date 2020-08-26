# MODIFIABLE PART
PROGRAM		:= libs
BUILDDIR 	:= build
OBJDIR 		:= $(BUILDDIR)/obj
BINDIR		:= $(BUILDDIR)/bin
DEPDIR		:= $(BUILDDIR)/deps
SRCDIR		:= 
LDLIBS 		:= -lasound -ldl
CPPFLAGS 	:= -I ..
CXXFLAGS 	:= -g3 -O3 \
				-Wall \
				-Wextra \
				-Wstrict-aliasing \
				-pedantic \
				-Werror \
				-Wunreachable-code \
				-Wcast-align \
				-Wcast-qual \
				-Wctor-dtor-privacy \
				-Winit-self \
				-Wlogical-op \
				-Wmissing-include-dirs \
				-Wnoexcept \
				-Woverloaded-virtual \
				-Wredundant-decls \
				-Wshadow \
				-Wsign-promo \
				-Wstrict-null-sentinel \
				-Wswitch-default \
				-Wundef \
				-Wno-unused \
				-Wno-parentheses \
				-fdiagnostics-show-option


# Write down all the source files here used to compile the final app
_SOURCES += SignalProcessor_Dummy.cpp

SOURCES += src/SignalProcessor/SignalProcessor_Dummy.cpp

OBJECTS += build/obj/SignalProcessor_Dummy.o

LIBRARIES += build/bin/libdummyimpl.so

# NON-MODIFIABLE PART
# DON'T TOUCH THE REST OF THIS SCIRPT, IF YOU DON'T KNOW, WHAT YOU ARE DOING
#
# additional flags to generate header dependencies files
DEPFLAGS 	= -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td
# postcompile command to move the newly generated dependency files
# so they won't get deleted after build finishes
POSTCOMPILE = mv -f $(DEPDIR)/$(*F).Td $(DEPDIR)/$(*F).d && touch $@

.PHONY: all clean distclean

# our default build target
all: $(PROGRAM)

distclean:
	rm -rf build

clean:
	rm -rf build/bin build/obj

# rule to build our program out of object files
# | $(BINDIR) - make sure there is a $(BINDIR) directory
# before linking object files into final binary
$(PROGRAM):  $(LIBRARIES) | $(BINDIR)

# rule to build objects out of sources
# to force rebuild not just on source files update, but also on header files, 
# we need to add the prerequisity on $(DEPDIR)/%.d
# as part of the build the compiler generates dependency files based on
# DEPFLAGS
# as the result appears to be intermediate, it would be erased at the end of
# build, thus a POSTCOMPILE action is introduced to move the generated files
# for future use
# make sure there is a $(DEPDIR) directory before building our objects
$(OBJDIR)/%.o: %.cpp $(DEPDIR)/%.d | $(DEPDIR)
	@echo "\nBuilding: $@"
	$(CXX) $(DEPFLAGS) $(CPPFLAGS) $(CXXFLAGS) -fPIC -c -o $@ $<
	$(POSTCOMPILE)

# TODO This recipe has been added in rush, needs to be cleaned up a bit maybe
$(BINDIR)/libdummyimpl.so: $(OBJDIR)/SignalProcessor_Dummy.o | $(BINDIR)
	@echo "\nBuilding: $@"
	$(CXX) -o $@ -shared $^

# make sure there is a $(OBJDIR) directory before building any object files
$(OBJECTS): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $@

$(BINDIR):
	mkdir -p $@

$(DEPDIR):
	mkdir -p $@

DEPFILES := $(_SOURCES:%.cpp=$(DEPDIR)/%.d)

# for very first built, there are no dep files. In such case
# provide a rule, which does nothing
$(DEPFILES):

# include the content of all generated DEPFILES containing
# dependancy on header files
include $(wildcard $(DEPFILES))