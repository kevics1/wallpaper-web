<template>
  <div class="home">
    <div class="banner">
      <el-carousel :interval="3000" type="card" height="400px">
        <el-carousel-item v-for="(item, index) in carousels" :key="index">
          <router-link :to="{ name: 'wallpaper', params: { id: item.id } }">
            <!-- <img :src="require('@/assets' + item.imageUrl)" alt="" style="width: 100%; height: 100%;"> -->
            <img :src="fullImageUrl(item.imageUrl)" alt="" style="width: 100%; height: 100%;">
          </router-link>
        </el-carousel-item>
      </el-carousel>
    </div>

    <div class="featured-wallpapers">
      <h2>热门壁纸</h2>
      <div class="wallpaper-grid">
        <!-- 使用v-for循环展示壁纸卡片 -->
        <WallpaperCard v-for="wallpaper in wallpapers.records" :key="wallpaper.id" :wallpaper="wallpaper" />
      </div>
    </div>

    <div class="pagination-container text-center">
      <el-pagination class="pagination" background layout="prev, pager, next" :total="wallpapers.total"
        :page-size="pageSize" :current-page="currentPage" @current-change="handleCurrentChange">
      </el-pagination>
    </div>
  </div>
</template>

<script>
// 导入组件
import WallpaperCard from '@/components/WallpaperCard.vue';
import { getWallpapers } from '@/api/wallpaper';
import { getCarousels } from '@/api/wallpaper';
import { BASE_IMG_URL } from '@/utils/baseUrl';

export default {
  name: 'Home',
  components: {
    WallpaperCard
  },
  data() {
    return {
      carousels: [],
      currentPage: 1,
      pageSize: 18,
      wallpapers: {},
    };
  },

  created() {
    this.getCarousels();
    this.fetchWallpapers();
  },
  methods: {
    fullImageUrl(imgUrl) {
      return BASE_IMG_URL + imgUrl;
    },
    async getCarousels() {
      await getCarousels().then(res => {
        this.carousels = res.data;
      });
    },
    async fetchWallpapers() {
      await getWallpapers(this.currentPage, this.pageSize, null, null).then(res => {
        this.wallpapers = res.data;
      });
    },
    async handleCurrentChange(newPage) {
      await getWallpapers(newPage, this.pageSize, null, null).then(res => {
        this.wallpapers = res.data;
      });
    }
  },
  computed: {
    // ...其他计算属性
    paginatedWallpapers() {
      let start = (this.currentPage - 1) * this.pageSize;
      let end = start + this.pageSize;
      return this.wallpapers.slice(start, end);
    },
    // ...
  },
};
</script>

<style scoped>
.home {
  /* 或其他要隐藏滚动条的元素的类 */
  overflow-y: auto;
  /* 为元素添加滚动功能，根据内容大小自动显示滚动条 */
  scrollbar-width: none;
  /* 针对 Firefox 隐藏滚动条 */
}

/* 针对 Chrome, Edge, Safari 等基于 WebKit 的浏览器隐藏滚动条 */
.home::-webkit-scrollbar {
  display: none;
  /* 使滚动条不显示 */
}

.banner {
  position: relative;
  width: 100%;
  /* max-width: 800px; 根据需求调整最大宽度 */
  margin: 0 auto;
}

/* 对 el-carousel-item 内的 img 元素进行样式定制 */
.banner .el-carousel__item>img {
  display: block;
  /* 确保图片作为块级元素显示 */
  width: 100%;
  /* 添加宽度以确保图片填充整个 Carousel Item 容器 */
  height: 100%;
  object-fit: cover;
  /* 保持图片的原始比例并填满容器，裁剪超出的部分 */
}

.el-carousel__item:nth-child(2n) {
  background-color: #99a9bf;
}

.el-carousel__item:nth-child(2n+1) {
  background-color: #d3dce6;
}

.featured-wallpapers h2 {
  font-size: 2.5em;
  /* 调整字体大小 */
  color: #333;
  /* 自定义字体颜色 */
  text-align: center;
  /* 文本居中对齐 */
  margin-bottom: 20px;
  /* 与下方内容保持一定距离 */
  text-shadow: 3px 3px 6px rgba(0, 0, 0, 0.7);
  /* 添加文本阴影效果 */
  font-weight: bold;
  /* 加粗字体 */
  letter-spacing: 1px;
  /* 字母间距 */
  line-height: 1.2;
  /* 行高 */
}

.wallpaper-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
  gap: 10px;
  justify-content: center;
  justify-items: center;
  /* width: 100%; */
}

.pagination-container {
  display: flex;
  justify-content: center;
  align-items: center;
  height: fit-content;
  /* 或者设置一个具体的高度值 */
  margin-bottom: 12px;
  /* 根据实际情况调整上边距 */
}
</style>