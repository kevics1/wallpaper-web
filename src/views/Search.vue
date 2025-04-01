<template>
  <div class="wallpaper-search-result">
    <el-card shadow="never" class="search-card">
      <div slot="header" class="search-header">
        <span>搜索内容：</span>
        <strong>{{ currentCategoryName }}</strong>
        <span class="separator">|</span>
        <span>查询到 {{ totalWallpapers }} 张壁纸</span>
      </div>

      <div class="wallpaper-grid">
        <!-- 使用v-for循环展示壁纸卡片 -->
        <WallpaperCard
          v-for="wallpaper in wallpapers.records"
          :key="wallpaper.id"
          :wallpaper="wallpaper"
        />
      </div>

      <!-- 如果需要分页功能，可以添加 Element UI 的 Pagination 组件 -->
      <el-pagination
        v-if="totalWallpapers > perPage"
        @size-change="handlePageSizeChange"
        @current-change="handlePageChange"
        :current-page="currentPage"
        :page-sizes="[10, 20, 30]"
        :page-size="perPage"
        layout="total, sizes, prev, pager, next, jumper"
        :total="totalWallpapers"
      />
    </el-card>
  </div>
</template>

<script>
import WallpaperCard from '@/components/WallpaperCard.vue';
import { getWallpapers } from '@/api/wallpaper';

export default {
  components: {
    WallpaperCard,
  },
  data() {
    return {
      searchContent: this.$route.query.searchQuery,
      wallpapers: {
        records: [],
        total: 0,
      },
      currentPage: 1,
      perPage: 10,
      currentCategoryId: this.$route.query.categoryId || null,
      currentCategoryName: this.$route.query.keyword || this.searchContent
    };
  },
  computed: {
    totalWallpapers() {
      return this.wallpapers.total;
    },
  },
  watch: {
    // 观察路由查询参数 searchQuery 的变化
    '$route.query.searchQuery': {
      immediate: true, // 组件加载时立即执行
      handler(newQuery, oldQuery) {
        if (newQuery !== oldQuery) {
          this.currentPage = 1; // 切换页大小时重置为第一页
          this.searchContent = newQuery;
          this.currentCategoryName = newQuery;
          this.getWallpapers();
        }
      }
    }
  },
  methods: {
    async getWallpapers(currentPage = 1, perPage = this.perPage) {
      const { data } = await getWallpapers(currentPage, perPage, this.currentCategoryId, this.searchContent);
      this.wallpapers = {
        records: data.records,
        total: data.total,
      };
    },

    handlePageSizeChange(newPageSize) {
      this.perPage = newPageSize;
      this.currentPage = 1; // 切换页大小时重置为第一页
      this.getWallpapers();
    },

    async handlePageChange(newPage) {
      this.currentPage = newPage;
      await this.getWallpapers(newPage, this.perPage);
    },
  },
  created() {
    // 页面加载时默认获取数据
    this.getWallpapers();
  },
};
</script>

<style scoped>

.wallpaper-search-result {
  padding: 1em;
  background: #f7f7f7; /* 背景色改为一个淡灰色以分离壁纸视觉页面 */
  overflow-y: auto;
  scrollbar-width: none;
}

.wallpaper-search-result ::-webkit-scrollbar {
  display: none;
}

.search-card {
  border-radius: 8px; /* 给卡片增加 border-radius 以圆滑边角 */
  overflow: hidden; /* 隐藏超出边界的内容 */
  box-shadow: 0 4px 6px rgba(0,0,0,0.1); /* 添加一个轻微的立体阴影 */
  transition: box-shadow 0.3s; /* 添加过渡效果，鼠标悬浮时提高立体感 */
}

.search-card:hover {
  box-shadow: 0 8px 12px rgba(0,0,0,0.2); /* 当鼠标悬浮在卡片上时，让卡片的立体感增加 */
}

.search-header {
  padding: 20px;
  background: white; /* 设置白色背景 */
  font-size: 16px;
  color: #333; /* 字体颜色改为深灰色以增加可读性 */
}

.separator {
  margin: 0 8px;
  color: #bbb; /* 分隔符颜色 */
}

.wallpaper-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(250px, 1fr));
  gap: 24px; /* 增加间距到24px */
  margin-top: 20px;
}

.el-pagination {
  padding: 20px 0; /* 分页的上下内边距增加，更加舒适 */
  display: flex;
  justify-content: center; /* 将分页组件居中 */
}

.el-pagination .el-pagination__item.is-active { /* 自定义当前页码的样式 */
  border-color: #409eff !important;
  background-color: #409eff !important;
  color: #fff !important;
}

.el-pagination button {
  border-radius: 50%; /* 将翻页按钮做成圆形 */
  width: 36px; /* 尺寸统一为36px */
  height: 36px; /* 尺寸统一为36px */

/* 默认字体可能略小，如果内容显示不清楚，可以适当调整 */
  font-size: 14px;
}

.el-pagination button:hover {
  background-color: #409eff;
  color: #fff;
}

.WallpaperCard {
  border-radius: 8px; /* 根据组件类名提前设置壁纸卡片的样式 */
  overflow: hidden; /* 隐藏超出边界的图片部分以确保边界的整洁 */
}
</style>