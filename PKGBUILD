pkgname=rogkbdsync
pkgver="$(git log --format="%h" -n1)"
pkgrel=1
pkgdesc='Workaround for GA503RM laptops, whose keyboard backlight controls dont work.'
arch=('any')
conflicts=()
license=('MIT')
depends=('hidapi')
srcdir='src'
makedepends=('gcc')
source=()
sha256sums=()

pkgver() {
    git log --format="%h" -n1
}

build() {
    g++ rogkbdsync.c $(pkg-config --cflags hidapi-hidraw) $(pkg-config --libs hidapi-hidraw) -o rogkbdsync
}

package() {
    install -Dm755 "$srcdir"/rogkbdsync "$pkgdir"/usr/bin/rogkbdsync
    install -Dm644 "$srcdir"/rogkbdsync.service "$pkgdir"/usr/lib/systemd/system/rogkbdsync.service
}
