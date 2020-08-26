$(OBJDIR)/%.o: $(SRCDIR)/AudioFrontEnd/%.cpp $(DEPDIR)/%.d | $(DEPDIR)
	@echo "\nBuilding: $@"
	$(CXX) $(DEPFLAGS) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<
	$(POSTCOMPILE)
