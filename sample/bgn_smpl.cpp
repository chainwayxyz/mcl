/*
	sample of somewhat homomorphic encryption(SHE) by BGN with prime order group
*/
#define PUT(x) std::cout << #x << "=" << (x) << std::endl;
#include <cybozu/benchmark.hpp>
#include <mcl/bn256.hpp>
#include <mcl/bgn.hpp>

#if CYBOZU_CPP_VERSION >= CYBOZU_CPP_VERSION_CPP11
#include <random>
std::random_device g_rg;
#else
#include <cybozu/random_generator.hpp>
cybozu::RandomGenerator g_rg;
#endif

typedef mcl::bgn::BGNT<mcl::bn256::BN, mcl::bn256::Fr> BGN;

using namespace mcl::bgn;
using namespace mcl::bn256;

void miniSample()
{
	// init library
	BGN::init();

	BGN::SecretKey sec;

	// init secret key by random_device
	sec.setByCSPRNG(g_rg);

	// set range to decode GT DLP
	sec.setRangeForGTDLP(1000);

	BGN::PublicKey pub;
	// get public key
	sec.getPublicKey(pub);

	const int N = 5;
	int a[] = { 1, 5, -3, 4, 6 };
	int b[] = { 4, 2, 1, 9, -2 };
	// compute correct value
	int sum = 0;
	for (size_t i = 0; i < N; i++) {
		sum += a[i] * b[i];
	}

	std::vector<BGN::CipherText> ca(N), cb(N);

	// encrypt each a[] and b[]
	for (size_t i = 0; i < N; i++) {
		pub.enc(ca[i], a[i], g_rg);
		pub.enc(cb[i], b[i], g_rg);
	}
	BGN::CipherText c;
	c.clearAsMultiplied(); // clear as multiplied before using c.add()
	// inner product of encrypted vector
	for (size_t i = 0; i < N; i++) {
		BGN::CipherText t;
		BGN::CipherText::mul(t, ca[i], cb[i]); // t = ca[i] * cb[i]
		c.add(t); // c += t
	}
	// decode it
	int m = sec.dec(c);
	// verify the value
	if (m == sum) {
		puts("ok");
	} else {
		printf("err correct %d err %d\n", sum, m);
	}
}

void usePrimitiveCipherText()
{
	// init library
	BGN::init();

	BGN::SecretKey sec;

	// init secret key by random_device
	sec.setByCSPRNG(g_rg);

	// set range to decode GT DLP
	sec.setRangeForGTDLP(1000);

	BGN::PublicKey pub;
	// get public key
	sec.getPublicKey(pub);

	int a1 = 1, a2 = 2;
	int b1 = 5, b2 = -4;
	BGN::CipherTextG1 c1, c2; // sizeof(CipherTextG1) = N * 2 ; N = 256-bit for CurveFp254BNb
	BGN::CipherTextG2 d1, d2; // sizeof(CipherTextG2) = N * 4
	pub.enc(c1, a1, g_rg);
	pub.enc(c2, a2, g_rg);
	pub.enc(d1, b1, g_rg);
	pub.enc(d2, b2, g_rg);
	c1.add(c2); // CipherTextG1 is additive HE
	d1.add(d2); // CipherTextG2 is additive HE
	BGN::CipherTextM cm; // sizeof(CipherTextM) = N * 12 * 4
	BGN::CipherTextM::mul(cm, c1, d1); // cm = c1 * d1
	cm.add(cm); // 2cm
	int m = sec.dec(cm);
	int ok = (a1 + a2) * (b1 + b2) * 2;
	if (m == ok) {
		puts("ok");
	} else {
		printf("err m=%d ok=%d\n", m, ok);
	}
}

int main()
	try
{
	miniSample();
	usePrimitiveCipherText();
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}