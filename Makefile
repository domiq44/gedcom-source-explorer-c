CC = gcc
CLANG_FORMAT ?= clang-format
CLANG_TIDY ?= clang-tidy
CPPCHECK ?= cppcheck
LINT_CHECKS ?= clang-analyzer-core.*,bugprone-*,-bugprone-multi-level-implicit-pointer-conversion,performance-*
CPPCHECK_CHECKS ?= warning,style,performance,portability
CFLAGS = -Wall -Wextra -g -Iinclude
BUILD_DIR = build
SRC_DIR = src
TEST_DIR = tests
FORMAT_SOURCES = $(wildcard $(SRC_DIR)/*.c $(TEST_DIR)/*.c include/*.h)
ANALYSIS_SOURCES = $(wildcard $(SRC_DIR)/*.c $(TEST_DIR)/*.c)
WEB_DIR = web
GEDCOM ?= gedcom-test.ged
BROWSER_TEST_GEDCOM = gedcom-test.ged
TARGET = $(BUILD_DIR)/gedcom_parser
REPORT_DIR = $(BUILD_DIR)/report
REPORT_HTML = $(REPORT_DIR)/index.html
BROWSER_TEST_DIR = $(BUILD_DIR)/browser-test
BROWSER_REPORT_HTML = $(BROWSER_TEST_DIR)/index.html
DIST_DIR = $(BUILD_DIR)/dist
DIST_NAME ?= gedcom-source-explorer-$(shell uname -s | tr '[:upper:]' '[:lower:]')-$(shell uname -m)
DIST_ROOT = $(DIST_DIR)/$(DIST_NAME)
DIST_ARCHIVE = $(DIST_DIR)/$(DIST_NAME).tar.gz
PARSER_TEST_TARGET = $(BUILD_DIR)/test_parser
CONTEXT_TEST_TARGET = $(BUILD_DIR)/test_gedcom_context
SOURCE_LIST_TEST_TARGET = $(BUILD_DIR)/test_source_list
STRING_UTILS_TEST_TARGET = $(BUILD_DIR)/test_string_utils
REPOSITORY_LIST_TEST_TARGET = $(BUILD_DIR)/test_repository_list
HTML_REPORT_TEST_TARGET = $(BUILD_DIR)/test_html_report
SOURCE_ORDER_TEST_TARGET = $(BUILD_DIR)/test_source_order
TEST_TARGETS = $(PARSER_TEST_TARGET) $(CONTEXT_TEST_TARGET) $(SOURCE_LIST_TEST_TARGET) $(STRING_UTILS_TEST_TARGET) $(REPOSITORY_LIST_TEST_TARGET) $(HTML_REPORT_TEST_TARGET) $(SOURCE_ORDER_TEST_TARGET)
LIB_SOURCES = $(SRC_DIR)/gedcom_parser.c $(SRC_DIR)/gedcom_context.c $(SRC_DIR)/gedcom_excerpt.c $(SRC_DIR)/source_list.c $(SRC_DIR)/repository_list.c $(SRC_DIR)/string_utils.c $(SRC_DIR)/source_order.c $(SRC_DIR)/report_data.c
APP_SOURCES = $(LIB_SOURCES) $(SRC_DIR)/html_report.c
MAIN_SOURCE = $(SRC_DIR)/main.c

.PHONY: all run report test test-browser dist format lint cppcheck clean clean-build clean-browser clean-all

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(APP_SOURCES) $(MAIN_SOURCE) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(APP_SOURCES) $(MAIN_SOURCE) -o $@

$(PARSER_TEST_TARGET): $(TEST_DIR)/test_parser.c $(LIB_SOURCES) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DSTRING_UTILS_TESTING $^ -o $@

$(CONTEXT_TEST_TARGET): $(TEST_DIR)/test_gedcom_context.c $(SRC_DIR)/gedcom_context.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(SOURCE_LIST_TEST_TARGET): $(TEST_DIR)/test_source_list.c $(SRC_DIR)/source_list.c $(SRC_DIR)/gedcom_excerpt.c $(SRC_DIR)/string_utils.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DSTRING_UTILS_TESTING $^ -o $@

$(STRING_UTILS_TEST_TARGET): $(TEST_DIR)/test_string_utils.c $(SRC_DIR)/string_utils.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(REPOSITORY_LIST_TEST_TARGET): $(TEST_DIR)/test_repository_list.c $(SRC_DIR)/repository_list.c $(SRC_DIR)/gedcom_excerpt.c $(SRC_DIR)/string_utils.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(HTML_REPORT_TEST_TARGET): $(TEST_DIR)/test_html_report.c $(SRC_DIR)/html_report.c $(SRC_DIR)/report_data.c $(SRC_DIR)/source_list.c $(SRC_DIR)/repository_list.c $(SRC_DIR)/gedcom_excerpt.c $(SRC_DIR)/string_utils.c $(WEB_DIR)/report.html | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_html_report.c $(SRC_DIR)/html_report.c $(SRC_DIR)/report_data.c $(SRC_DIR)/source_list.c $(SRC_DIR)/repository_list.c $(SRC_DIR)/gedcom_excerpt.c $(SRC_DIR)/string_utils.c -o $@

$(SOURCE_ORDER_TEST_TARGET): $(TEST_DIR)/test_source_order.c $(SRC_DIR)/source_order.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

run: $(TARGET)
	./$(TARGET) "$(GEDCOM)"

report: $(TARGET) $(WEB_DIR)/report.html $(WEB_DIR)/report.css $(WEB_DIR)/report.js
	mkdir -p $(REPORT_DIR)
	cp $(WEB_DIR)/report.css $(WEB_DIR)/report.js $(REPORT_DIR)/
	./$(TARGET) --html "$(GEDCOM)" "$(REPORT_HTML)"

test: $(TEST_TARGETS)
	@status=0; \
	for test_binary in $(TEST_TARGETS); do \
		test_name=$${test_binary##*/}; \
		printf '\n=== %s ===\n' "$$test_name"; \
		if "$$test_binary"; then \
			printf '[PASS] %s\n' "$$test_name"; \
		else \
			printf '[FAIL] %s\n' "$$test_name"; \
			status=1; \
		fi; \
	done; \
	exit $$status

test-browser: $(TARGET) $(BROWSER_TEST_GEDCOM) $(WEB_DIR)/report.html $(WEB_DIR)/report.css $(WEB_DIR)/report.js
	mkdir -p $(BROWSER_TEST_DIR)
	cp $(WEB_DIR)/report.css $(WEB_DIR)/report.js $(BROWSER_TEST_DIR)/
	./$(TARGET) --html "$(BROWSER_TEST_GEDCOM)" "$(BROWSER_REPORT_HTML)"
	npm run test:browser

dist: $(TARGET) gedcom-test.ged README.md DIST_README.md $(WEB_DIR)/report.html $(WEB_DIR)/report.css $(WEB_DIR)/report.js
	rm -rf "$(DIST_ROOT)"
	mkdir -p "$(DIST_ROOT)/web" "$(DIST_ROOT)/build/report"
	cp $(TARGET) "$(DIST_ROOT)/gedcom-source-explorer"
	cp gedcom-test.ged DIST_README.md "$(DIST_ROOT)/"
	cp README.md "$(DIST_ROOT)/README-project.md"
	cp $(WEB_DIR)/report.html $(WEB_DIR)/report.css $(WEB_DIR)/report.js "$(DIST_ROOT)/web/"
	cp $(WEB_DIR)/report.css $(WEB_DIR)/report.js "$(DIST_ROOT)/build/report/"
	tar -czf "$(DIST_ARCHIVE)" -C "$(DIST_DIR)" "$(DIST_NAME)"
	@printf 'Distribution créée : %s\n' "$(DIST_ARCHIVE)"

format:
	$(CLANG_FORMAT) -i -style='{BasedOnStyle: LLVM, IndentWidth: 4}' $(FORMAT_SOURCES)

lint:
	$(CLANG_TIDY) -quiet -checks='$(LINT_CHECKS)' -warnings-as-errors='*' $(ANALYSIS_SOURCES) -- -std=c11 $(CFLAGS) -DSTRING_UTILS_TESTING

cppcheck:
	$(CPPCHECK) --quiet --inline-suppr --enable=$(CPPCHECK_CHECKS) --error-exitcode=1 --language=c --std=c11 -DSTRING_UTILS_TESTING -I include $(ANALYSIS_SOURCES)

clean: clean-build

clean-build:
	rm -rf $(BUILD_DIR) gedcom_parser test_runner *.o

clean-browser:
	rm -rf node_modules test-results playwright-report

clean-all: clean-build clean-browser
