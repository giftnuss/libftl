#include <pona/stdio>
#include <pona/string>

namespace pona
{

int main(int argc, char** argv)
{
	if (argc != 2) {
		print("Usage: %% [FILE]\n", File(argv[0]).name());
		return 0;
	}
	
	File file(argv[1], File::Read);
	UString text(file.size());
	file.read(text.data(), text.size());
	if (!text.valid()) {
		print("Input data is not conforming to UTF8 encoding.\n");
		return 1;
	}
	if (text.empty()) {
		print("Input file empty.\n");
		return 2;
	}
	
	print("text = \"%%\"\n", text.data());
	
	int nf = 0, nb = 0; // forward, backward character count
	for (UString::Index i = text.first(); i.valid(); ++i) ++nf;
	for (UString::Index i = text.last(); i.valid(); --i) ++nb;
	
	/*print("======================\n");
	
	for (UString::Index i = text.last(); i.valid(); --i) {
		uchar_t ch = text.get(i);
		print("text[0x%%] = %% / '%%'\n", i.pos() - i.data(), ch, Char(ch));
	}
	
	print("----------------------\n");
	
	for (UString::Index i = text.first(); i.valid(); ++i) {
		uchar_t ch = text.get(i);
		print("text[0x%%] = %% / '%%'\n", i.pos() - i.data(), ch, Char(ch));
	}
	
	print("nf, nb = %%, %%\n", nf, nb);
	if (nf != nb) {
		print("Test failed: nf != nb\n");
		return 3;
	}
	
	print("----------------------\n");*/
	
	{
		UString s("123x");
		const char* pattern = "x";
		UString::Index i = s.find(pattern);
		print("Find \"%%\" in \"%%\": %% (valid = %%)\n", pattern, s.data(), i.pos() - i.data(), i.valid());
		/*print("s == UString(s.split(pattern), pattern): %%\n", s == UString(s.split(pattern), pattern));
		print("s.split(pattern)->length() = %%\n", s.split(pattern)->length());
		print("UString(s.split(pattern), pattern) = '%%'\n", UString(s.split(pattern), pattern).data());*/
	}
	
	Ref<UStringList, Owner> lines = text.split("\n");
	print("Number of lines: %%\n", lines->length());
	for (int i = 0; i < lines->length(); ++i)
		print("%%: '%%' (%%)\n", i, lines->get(i).data(), lines->get(i).size());
	
	return 0;
}

} // namespace pona

int main(int argc, char** argv)
{
	return pona::main(argc, argv);
}