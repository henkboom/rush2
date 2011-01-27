BUILD_DIR := build
CC := clang

SRC :=
PACKAGES :=
CFLAGS := -Wall -g -O0 -std=c99
LDFLAGS := -lglfw -lGL -lGLU -lm

include project.mk
include $(PACKAGES:%=%/package.mk)

TARGET_EXE := $(PROJECT_NAME)

OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRC))
DEPS := $(patsubst %.c, $(BUILD_DIR)/%.P, $(SRC))

$(TARGET_EXE): $(OBJS)
	@echo linking $@...
	@$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	@echo building $@...
	@mkdir -p `dirname $@`
	@$(CC) -MD -o $@ $< -c $(CFLAGS)
	@cp $(BUILD_DIR)/$*.d $(BUILD_DIR)/$*.P;
	@sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	     -e '/^$$/ d' -e 's/$$/ :/' < $(BUILD_DIR)/$*.d >> $(BUILD_DIR)/$*.P
	@rm -f $(BUILD_DIR)/$*.d

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET_EXE)

-include $(DEPS)
