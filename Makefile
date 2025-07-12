CCOMP="gcc"

clean:
	rm -f sshman

build:
	$(CCOMP) picker.c -o sshman -lncurses

buildDebug:
	$(CCOMP) picker.c -o sshman -lncurses -D DEBUG=1

run: buildDebug
	@echo "Running"
	./sshman
	
install: build
	@echo "Installing sshman"
	@./install.sh