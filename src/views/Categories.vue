<template>
  <el-container style="height: 100vh;">
    <el-aside width="200px">
      <el-menu class="el-menu-vertical-demo" @select="handleSelectFirstLevel">
        <el-menu-item v-for="item in firstLevelCategories" :key="item.id" :index="item.id.toString()"
          @click="handleSelectFirstLevel(item.id)">
          {{ item.name }}
        </el-menu-item>
      </el-menu>
    </el-aside>

    <el-aside width="200px">
      <el-menu class="el-menu-vertical-demo" @select="handleSelectSecondLevel">
        <el-menu-item v-for="item in secondLevelCategories" :key="item.id" :index="item.id.toString()">
          {{ item.name }}
        </el-menu-item>
      </el-menu>
    </el-aside>

    <el-container>
      <el-header style="text-align: center;">
        <el-input v-model="searchText" placeholder="搜索壁纸..." prefix-icon="el-icon-search" class="search-input">

        </el-input>
        <el-button type="primary" @click="onSearch">搜索</el-button>
      </el-header>
      <el-main>
        <div v-for="category in thirdLevelCategories" :key="category.id" class="category-row">
          <router-link :to="{ name: 'search', query: { categoryId: category.id, keyword: category.name } }" class="category-card">
            <el-card class="category-card" @click.native="goToCategoryDetail(category.id)">
              <div class="category-content-and-stats">
                <div class="category-content">
                  {{ category.name }}
                </div>
                <div class="card-stats">
                  <span class="stat-item">
                    <i class="el-icon-view"></i>
                    1200
                  </span>
                  <span class="stat-item">
                    <i class="el-icon-picture"></i>
                    100
                  </span>
                </div>
              </div>
              <el-breadcrumb class="breadcrumb">
                <el-breadcrumb-item>{{ category.breadcrumb }}</el-breadcrumb-item>
                <!-- <el-breadcrumb-item>{{ category.breadcrumb }}</el-breadcrumb-item> -->
              </el-breadcrumb>
            </el-card>
          </router-link>

        </div>

        <el-pagination v-if="thirdLevelCategories.length > 0" class="pagination"
          layout="prev, pager, next, jumper, ->, total" :total="totalItems" :current-page="currentPage"
          :page-size="pageSize" @current-change="onChangePage" />
      </el-main>
    </el-container>
  </el-container>
</template>

<script>
import { getCategory } from '@/api/wallpaper';

export default {
  data() {
    return {
      firstLevelCategories: [],
      secondLevelCategories: [],
      thirdLevelCategories: [],
      searchText: '',
      selectedSecondLevelId: null,
      selectedThirdLevelId: null,
      currentPage: 1,
      pageSize: 5,
      totalItems: 0,
    };
  },
  computed: {
    filteredWallpapers() {
      if (this.selectedSecondLevelId) {
        return this.wallpapers.filter(
          (wp) => wp.categoryId === this.selectedSecondLevelId
        );
      } else if (this.searchText) {
        return this.wallpapers.filter((wp) =>
          wp.name.toLowerCase().includes(this.searchText.toLowerCase())
        );
      }
      return this.wallpapers;
    },
  },
  created() {
    this.getCategories(1, 50, null, null, 0, 1);
  },
  methods: {
    handleSelectFirstLevel(id) {
      this.getCategories(1, 50, null, null, id, 2);
      // 清空搜索框
      this.searchText = '';
      // 清空三级列表
      this.selectedSecondLevelId = null;
    },
    handleSelectSecondLevel(id) {
      this.searchText = '';
      this.selectedThirdLevelId = id;
      this.getCategories(1, this.pageSize, null, null, id, 3);
    },
    onSearch() {
      if (this.searchText) {
        this.getCategories(1, this.pageSize, null, this.searchText, null, 3);
      } else {
        this.$errorMessage('请输入搜索内容');
      }
    },
    onChangePage(newPage) {
      this.currentPage = newPage;
      if (this.searchText) {
        this.getCategories(newPage, this.pageSize, null, this.searchText, null, 3);
      } else {
        this.getCategories(newPage, this.pageSize, null, null, this.selectedThirdLevelId, 3);
      }

    },
    async getCategories(page, pageSize, id, categoryName, parentId, type) {
      const res = await getCategory(page, pageSize, id, categoryName, parentId);
      if (res.code == 200) {
        if (type == 1) {
          this.firstLevelCategories = res.data.records;
        } else if (type == 2) {
          this.secondLevelCategories = res.data.records;
        } else if (type == 3) {
          this.thirdLevelCategories = res.data.records;
          this.totalItems = res.data.total;
        }
      } else {
        this.$errorMessage(res.msg);
      }
    },
  },
};
</script>

<style scoped lang="scss">
.app-container {
  padding: 20px;
}

.search-container {
  text-align: center;
  margin-bottom: 20px;
}

.search-input {
  margin-top: 15px;
  width: 50%;
}

.category-row {
  margin-bottom: 10px;
}

.breadcrumb {
  margin-bottom: 5px;
}

.category-card {
  cursor: pointer;
  border-radius: 4px; // 添加圆角
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1); // 添加阴影效果

}

.category-content {
  font-size: 16px; // 调整字体大小
  font-weight: 500; // 加粗标题
}

.category-content-and-stats {
  // display: flex;
  // flex-direction: column;
  // justify-content: space-between;
  height: auto; // 取消固定高度，以便内容自适应
}

// 新增样式：用于对浏览量和壁纸数量进行布局和样式调整
.card-stats {
  display: flex;
  justify-content: flex-end;
  align-items: center;
  margin-top: 7px;
  // 调整顶部间距以适应新的布局
}

.stat-item {
  margin-left: 10px;
  font-size: 14px;
  color: #8492a6;

  i {
    margin-right: 5px;
    color: #bfc9d6;
  }
}

// Aside component base
.el-aside {
  background-color: #f5f5f5; // 浅灰色背景
}

// Base Menu styles
.el-menu {
  border-right: 1px solid #eaeaea; // 添加右侧边框颜色
}

// Menu items normal state
.el-menu-item {
  padding-left: 20px; // 增加左侧内边距
  line-height: 50px; // 设置行高和高度，使得菜单项更加易读和点击
  transition: background-color 0.2s ease; // 平滑过渡效果

  &:hover {
    background-color: #e6e6e6; // 悬停时更改背景色
  }
}

// Menu vertical demo class, may have its current specific styles
.el-menu-vertical-demo:not(.el-menu--collapse) {
  border: none;
}

// First Level Aside menu customization
.el-aside:first-child {
  .el-menu {
    background-color: #ffffff; // 定义一个更深或者更适合的背景色
  }
}

// Customizing all Aside items
.el-aside {
  .el-menu-item {
    font-weight: 500; // 加粗字体可能使得项更容易区分
    color: #333; // 字体颜色，使得文本具备足够的对比度
  }
}

// Customizing active state for menu items
.el-menu-item.is-active {
  // background-color: #def1ff; // 高亮选中项，换成适配主题的颜色
  color: #0bb8fd; // 设置一个符合高亮主题颜色的颜色
}

.pagination {
  text-align: center;

  .el-pagination__jump {
    margin-left: 10px; // 调整快速跳转间距
  }
}
</style>