all:	testplugin

gold:
	rm -rf Reference/00_polardesigner_all_formats_reaper_reference.wav
	./00_polardesigner_all_formats_test.sh --create-gold-reference

testplugin:
	./00_polardesigner_all_formats_test.sh 

reset:	clean	gold	testplugin

clean:
	rm -rf Results/*

