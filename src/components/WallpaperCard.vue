<template>
  <div class="wallpaper=container">
    <router-link :to="{ name: 'wallpaper', params: { id: wallpaper.id } }">
      <div class="wallpaper-card" @mouseenter="toggleHover(true)" @mouseleave="toggleHover(false)">
        <div class="wallpaper-image-wrapper">
          <img :src="fullImageUrl(wallpaper.imageUrl)" :alt="wallpaper.title" class="wallpaper-image" />
        </div>
        <div class="info" v-if="hovering">
          <h3>{{ wallpaper.title }}</h3>
          <p>{{ wallpaper.description }}</p>
        </div>
      </div>
    </router-link>
  </div>

</template>

<script>
import { BASE_IMG_URL } from '@/utils/baseUrl';
export default {
  name: 'WallpaperCard',
  props: {
    wallpaper: {
      type: Object,
      required: true,
    },
  },
  data() {
    return {
      hovering: false,
    };
  },
  methods: {
    toggleHover(state) {
      this.hovering = state;
    },
    fullImageUrl(imgUrl) {
      return BASE_IMG_URL + imgUrl;
    },
  },
};
</script>

<style scoped>
.wallpaper-card {
  display: inline-block;
  margin: 10px;
  border-radius: 8px;
  overflow: hidden;
  box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
  transition: transform 0.2s ease, box-shadow 0.2s ease;
  cursor: pointer;
  position: relative;
  min-width: 300px;
}

.wallpaper-card:hover {
  transform: scale(1.05);
  box-shadow: 0 6px 12px rgba(0, 0, 0, 0.2);
}

.wallpaper-image-wrapper {
  height: 0;
  padding-top: 56.25%;
  /* 16:9 aspect ratio */
  position: relative;
}

.wallpaper-image {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: auto;
  object-fit: cover;
}

.info {
  padding: 10px;
  background-color: rgba(0, 0, 0, 0.7);
  color: white;
  transition: opacity 0.3s ease;
  opacity: 0;
  position: absolute;
  bottom: 0;
  left: 0;
  right: 0;
}

.wallpaper-card:hover .info {
  opacity: 1;
}

.info>h3 {
  margin: 0;
  font-size: 16px;
}

.info>p {
  font-size: 14px;
  margin-top: 5px;
}
</style>