<template>
  <header
    class="w-full text-center py-6 bg-gradient-to-b from-gray-900/70 to-transparent backdrop-blur-sm"
  >
    <!-- Judul -->
    <h1 class="text-3xl md:text-4xl font-bold tracking-wide text-white drop-shadow-lg">
      Jam Digital <span class="text-indigo-400">Control</span>
    </h1>

    <!-- Garis tipis dekoratif -->
    <div class="mx-auto mt-3 h-0.5 w-1/3 bg-gradient-to-r from-transparent via-indigo-400 to-transparent"></div>

    <!-- Teks berjalan -->
    <div class="overflow-hidden mt-5">
      <p
        class="whitespace-nowrap animate-marquee text-base md:text-lg text-gray-300 tracking-wide font-medium"
        style="animation: marquee 25s linear infinite;"
      >
        Sistem jam digital pintar ini dapat menampilkan waktu, suhu, dan teks dinamis — Website kontrol dibangun oleh Informatika A
      </p>
    </div>
  </header>
</template>

<style>
@keyframes marquee {
  0% {
    transform: translateX(100%);
  }
  100% {
    transform: translateX(-100%);
  }
}
</style>
