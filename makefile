.PHONY: clean

editor: editor.c
	gcc editor.c -o editor

clean:
	rm editor

