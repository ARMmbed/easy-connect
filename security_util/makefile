# Generate STATIC library.
TARGET = libsecurity.a

INC			=	-I./include \
				-I../mbedtls \
				-I../mbedtls/configs \
				-L../mbedtls/library


CC       = g++
# compiling flags here
CFLAGS   = -O3 -Wall -fPIC 

LINKER   = ar rvs

# linking flags here
LFLAGS   = 
BIT_TYPE=64

# change these to set the proper directories where each files shoould be

SRCDIR   = src
OBJDIR   = x86/$(BIT_TYPE)/obj
LIBDIR   = x86/$(BIT_TYPE)/lib

SOURCES  := $(wildcard $(SRCDIR)/*.cpp)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
rm       = rm -f

$(LIBDIR)/$(TARGET): $(OBJECTS)
	@$(LINKER) $@ $(LFLAGS) $(OBJECTS)
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@$(CC) $(INC) $(CFLAGS) -c $< -o $@ 
	@echo "Compiled "$<" successfully!"

.PHONEY: clean
clean:
	@$(rm) $(OBJECTS)
	@$(rm) $(LIBDIR)/$(TARGET)
	@echo "Cleanup complete!" 

.PHONEY: remove
remove: clean
	@$(rm) $(LIBDIR)/$(TARGET)
	@echo "Executable removed!"
