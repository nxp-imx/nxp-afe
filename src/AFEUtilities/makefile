$(OBJDIR)/%.o: $(SRCDIR)/AFEUtilities/%.cpp $(DEPDIR)/%.d | $(DEPDIR)
	@echo "\nBuilding: $@"
	$(CXX) $(DEPFLAGS) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<
	$(POSTCOMPILE)
