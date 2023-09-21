pkgname=rogkbdsync
pkgver=fbd36fe
pkgrel=1
pkgdesc='Workaround for GA503RM laptops, whose keyboard backlight controls dont work,'
arch=('any')
conflicts=()
license=('MIT')
depends=('hidapi')
makedepends=('gcc')
source=(
    'main.c'
    'rogkbdsync.service'
)
sha256sums=(
    'bacf513e4c043b6bf467fa88f7be37db85d49a5b237407da14c389b7ab44fe6b'
    '51a6680ab25f70cd0d7ed8648bb9259ddb81287ddf001d8268f2e463c77a34d5'
)

pkgver() {
    git log --format="%h" -n1
}

build() {
    g++ main.c $(pkg-config --cflags hidapi-hidraw) $(pkg-config --libs hidapi-hidraw) -o rogkbdsync
}

package() {
    install -Dm755 "$srcdir"/rogkbdsync "$pkgdir"/usr/bin/rogkbdsync
    install -Dm644 "$srcdir"/rogkbdsync.service "$pkgdir"/usr/lib/systemd/system/rogkbdsync.service
}
