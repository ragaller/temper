temper: temper.c
	@gcc $< -l hidapi-hidraw -o $@

release: temper
	@cp temper temper-linux-$$(git tag -l --points-at HEAD | cut -d/ -f2)-$$(uname -m) \
		&& strip temper-linux-$$(git tag -l --points-at HEAD | cut -d/ -f2)-$$(uname -m) \
		&& sha256sum temper-linux-$$(git tag -l --points-at HEAD | cut -d/ -f2)-$$(uname -m) > temper-linux-$$(git tag -l --points-at HEAD | cut -d/ -f2)-$$(uname -m).sha256sum

clean:
	@rm -f temper temper-* *~ .*~

