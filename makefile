CLASS=argparse
COMBINED=$(CLASS)-combined.h

.PHONY: $(CLASS)
$(CLASS): SHELL:=/bin/bash
$(CLASS):
	@rm -f $(COMBINED)
	@while IFS= read -r line; do\
		[[ $$line == "};"* ]] && break;\
		printf "%s\n" "$$line" >> $@;\
	done < $(CLASS).h
	@echo >> $@
	@found=0
	@while IFS= read -r line; do\
		[[ $$line == "// ==="* ]] && found=1;\
		[[ found -eq 0 ]] && continue;\
		printf "\t%s\n" "$${line/argparse::/}" >> $@;\
	done < $(CLASS).cpp
	@printf "\t//}}}1\n};\n\n#endif//ARGUMENT_PARSER" >> $@
	@echo "Target $(COMBINED) successfully created."
