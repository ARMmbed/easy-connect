# Generate STATIC library.
TARGET = libsecurity.a

INC			=	-I./include \
				-I../mbedtls/include \
				-I../mbedtls/configs \
				-L../mbedtls/library


CC       = g++
# compiling flags here
CFLAGS   = -O3 -Wall -fPIC 

LINKER   = ar rvs

# linking flags here
LFLAGS   = 

# change these to set the proper directories where each files shoould be

SRCDIR   = src
OBJDIR   = obj
LIBDIR   = lib

SOURCES  := $(wildcard $(SRCDIR)/*.cpp)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
rm       = rm -f

$(LIBDIR)/$(TARGET): $(OBJECTS)
	@$(LINKER) $@ $(LFLAGS) $(OBJECTS)
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@$(CC) $(INC) $(CFLAGS) -c $< -o $@ -lmbedx509 -lmbedtls -lmbedcrypto
	@echo "Compiled "$<" successfully!"

	# -lmbedx509 -lmbedtls -lmbedcrypto
.PHONEY: clean
clean:
	@$(rm) $(OBJECTS)
	@echo "Cleanup complete!" 
	@echo $(OBJECTS)

.PHONEY: remove
remove: clean
	@$(rm) $(LIBDIR)/$(TARGET)
	@echo "Executable removed!"
