<template>
  <div class="wallpaper-detail-container" style="display: flex; ">
    <div class="info-section" style="width: 25%; background-color: #303133; color: #ffffff;">
      <h2 style="text-align: center;">{{ wallpaper.title }}</h2>
      <div class="meta-info info-top glass-effect">
        <span>分类：</span>
        <el-tag v-for="(tag, index) in wallpaper.categories" :key="index" size="mini" :type="getRandomTagType()">{{
          tag.name }}</el-tag>
        <br>
        <span>分辨率: </span>
        {{ wallpaper.resolution }}

        <p>上传者: {{ wallpaper.author }}</p>
      </div>

      <br>

      <div class="statistic-section">
        <el-row :gutter="22.5">
          <el-col :span="4.5">
            <div>
              <el-statistic class="el-statictic" group-separator="," :value="wallpaper.wallpaperStatisticsVO.viewNum"
                title="浏览数"></el-statistic>
            </div>
          </el-col>
          <el-col :span="4.5">
            <div>
              <el-statistic class="el-statictic" :value="wallpaper.wallpaperStatisticsVO.downloadNum" title="下载数">
              </el-statistic>
            </div>
          </el-col>
          <el-col :span="4.5">
            <div>
              <el-statistic class="el-statictic" group-separator="," decimal-separator="."
                :value="wallpaper.wallpaperStatisticsVO.likeNum" title="点赞数">
              </el-statistic>
            </div>
          </el-col>
          <el-col :span="4.5">
            <div>
              <el-statistic class="el-statictic" :value="wallpaper.wallpaperStatisticsVO.favorNum" title="收藏数">
              </el-statistic>
            </div>
          </el-col>
          <el-col :span="4.5">
            <div>
              <el-statistic class="el-statictic" :value="wallpaper.wallpaperStatisticsVO.commentNum" title="评论数">
              </el-statistic>
            </div>
          </el-col>
        </el-row>
      </div>

      <div class="comments-section info-section">
        <h3 style="padding-left: 10px;">评论区</h3>
        <div class="detail-comments">
          <!-- 可滚动的评论列表容器 -->
          <div class="comment-list-container" style="max-height: 500px; overflow-y: scroll;">
            <div v-if="!comments.length">
              <el-empty description="暂无评论，快来抢沙发~"></el-empty>
            </div>

            <ul class="comment-list" v-if="comments.length">
              <li class="comment-item" v-for="comment in comments" :key="comment.id">
                <div class="comment-avatar">
                  <!-- 用户头像 -->
                  <img
                    :src="comment.userAvatar == null ? 'https://cube.elemecdn.com/0/88/03b0d39583f48206768a7534e55bcpng.png' : comment.userAvatar"
                    alt="User Avatar" class="avatar-img">
                </div>
                <div class="comment-content">
                  <span class="comment-user">{{ comment.userName == null ? '匿名用户' : comment.userName }} : </span>
                  <span class="comment-text">{{ comment.content }}</span>
                  <p class="comment-date">{{ comment.commentTime }}</p>
                </div>
              </li>
            </ul>
          </div>
          <!-- 固定在左下角的评论输入框 -->
          <div class="comment-form-container">
            <form class="comment-form">
              <textarea v-model="newCommentText" placeholder="写下你的评论..." class="comment-input"></textarea>
            </form>
            <button type="submit" class="comment-submit" @click="submitComment">提交</button>
          </div>
        </div>
      </div>
    </div>


    <div class="image-section" :style="{ backgroundImage: 'url(' + fullImageUrl(wallpaper.url) + ')' }">
      <div class="button-container" style="position: absolute; bottom: 20px; left: 50%; transform: translateX(-50%);">
        <el-button size="large" type="primary" icon="el-icon-zoom-in" @click="handleEnlarge">放大</el-button>
        <el-button size="large" type="primary" icon="el-icon-check" @click="handleLike">点赞</el-button>
        <el-button size="large" type="primary" icon="el-icon-collection" @click="handleCollect">收藏</el-button>
        <!-- <el-button size="large" type="primary" icon="el-icon-share" @click="handleShare">分享</el-button> -->
        <el-button size="large" type="primary" icon="el-icon-download" @click="handleDownload">下载</el-button>
      </div>

      <!-- 点击放大按钮后显示的大图 -->
      <transition name="fade">
        <div v-if="showFullImageModal" class="full-image-container">
          <img :src="fullImageUrl(wallpaper.url)" class="full-image" alt="Full Image" @click="hideFullImage" />
        </div>
      </transition>
    </div>
  </div>
</template>


<script>
import { getComments } from '@/api/comment';
import { getWallpaperDetail } from '@/api/wallpaper';
import { operateWallpaper } from '@/api/wallpaper';
import { sendComment } from '@/api/comment';
import { BASE_IMG_URL } from '@/utils/baseUrl';

export default {
  name: 'wallpaper',
  data() {
    return {
      // 创建数据属性以存储获取的数据
      wallpaper: null,
      comments: [],
      newCommentText: '',
      showFullImageModal: false,
      operateResult: false
    };
  },
  created() {
    this.getWallpaper();
    this.getComments(1, 10);
    this.wallpaper.tags.forEach((tag) => {
      this.$set(tag, 'tagType', this.getRandomTagType());
    });
  },
  watch: {
    '$route'(to, from) {
      // 当路由改变时，重新获取数据
      this.getWallpaper();
      this.getComments(1, 10);
    }
  },
  methods: {
    fullImageUrl(imgUrl) {
      return BASE_IMG_URL + imgUrl;
    },
    async getComments(page, pageSize) {
      // 获取参数id
      const id = this.$route.params.id;

      // 调用接口获取评论数据
      await getComments(page, pageSize, id).then((res) => {
        this.comments = res.data.records;
      });
    },
    async getWallpaper() {
      const id = this.$route.params.id;
      await getWallpaperDetail(id).then((res) => {
        if (res.code == 200) {
          this.wallpaper = res.data;
        } else {
          this.$errorMessage(res.msg)
        }
      })
    },
    async sendComment(text) {
      const id = this.$route.params.id;
      // TODO 暂无用户管理
      await sendComment({ wallpaperId: id, content: text }).then((res) => {
        if (res.code == 200) {
          this.$successMessage('评论成功');
          this.wallpaper.wallpaperStatisticsVO.commentNum++;
          this.comments.unshift(res.data);
          this.newCommentText = '';
        } else {
          this.$errorMessage(res.msg)
        }
      });
    },
    getRandomTagType() {
      const tagTypes = ['success', 'info', 'warning', 'danger', 'primary'];
      return tagTypes[Math.floor(Math.random() * tagTypes.length)];
    },
    submitComment() {
      this.sendComment(this.newCommentText);
    },
    handleEnlarge() {
      // 放大功能的实现
      this.showFullImageModal = true;
    },
    hideFullImage() {
      this.showFullImageModal = false;
    },
    async handleLike() {
      // 点赞功能的实现
      await operateWallpaper({ wallpaperId: this.$route.params.id, type: 3 }).then((res) => {
        if (res.code == 200) {
          this.wallpaper.wallpaperStatisticsVO.likeNum++;
          this.$successMessage(res.msg);
        } else {
          this.$errorMessage(res.msg);
        }
      })
    },
    async handleCollect() {
      // 收藏功能的实现
      await operateWallpaper({ wallpaperId: this.$route.params.id, type: 2 }).then((res) => {
        if (res.code == 200) {
          this.wallpaper.wallpaperStatisticsVO.favorNum++;
          this.$successMessage(res.msg);
        } else {
          this.$errorMessage(res.msg);
        }
      })
    },
    handleShare() {
      // 分享功能的实现
    },
    async handleDownload() {
      await operateWallpaper({ wallpaperId: this.$route.params.id, type: 4 }).then((res) => {
        if (res.code == 200) {
          const url = BASE_IMG_URL + this.wallpaper.url;

          // 创建一个隐藏的可下载链接
          const aLink = document.createElement('a');

          // Fetch the image and create a Blob
          fetch(url)
            .then(response => response.blob())  // 获取图片并转为 Blob
            .then(blob => {
              // 创建一个新的 URL 对象指向 Blob
              const objectURL = URL.createObjectURL(blob);
              aLink.href = objectURL;  // 设置为 Blob 的 URL

              // 设置下载文件名
              aLink.download = this.wallpaper.title + '.' + this.getExtensionFromUrl(this.wallpaper.url);

              // 触发点击
              document.body.appendChild(aLink);
              aLink.click();

              // 下载完成后移除
              document.body.removeChild(aLink);

              // 释放 Object URL
              URL.revokeObjectURL(objectURL);

              this.wallpaper.wallpaperStatisticsVO.downloadNum++;
              this.$successMessage(res.msg);
            })
            .catch(err => {
              this.$errorMessage('下载失败: ' + err);
            });
        } else {
          this.$errorMessage(res.msg);
        }
      })
    },
    // 从URL获取扩展名的方法
    getExtensionFromUrl(url) {
      return url.substring(url.lastIndexOf('.') + 1);
    },
  },
};
</script>

<style lang="scss" scoped>
html,
body {
  margin: 0;
  padding: 0;
  height: 100%;
  overflow: hidden;
}

.glass-effect {
  position: relative;

  &::before {
    content: "";
    position: absolute;
    bottom: 0;
    left: 0;
    width: 100%;
    height: 120px; // 调整高度以适应设计需求
    border-radius: 20px; // 圆角大小根据设计需求调整
    background: linear-gradient(to bottom, rgba(255, 255, 255, 0), rgba(255, 255, 255, 0.2)); // 从透明渐变到半透明
  }
}

.statistic-section {
  display: flex;
  justify-content: center;
  /* 水平居中 */
  align-items: center;
  /* 垂直居中 */
  margin-left: -40px;
}

.comment-list {
  position: relative;
  // margin-bottom: -30px; /* 确保评论列表高度至少高于评论表单的高度 */
  /* 或者，如果知道具体高度：
  padding-bottom: calc(100% - 200px); // 假设.comment-form-container的高度为200px
  */
  // height: calc(50vh - 50px);
  list-style: none;
  padding: 0;
}

.comment-item {
  display: flex;
  margin-bottom: 15px;
}

.comment-avatar {
  margin-right: 10px;
}

.avatar-img {
  width: 50px;
  height: 50px;
  border-radius: 50%;
}

.comment-content {
  background: #ffffff;
  padding-left: 10px;
  border-radius: 8px;
  flex: 1;
}

.comment-user {
  font-weight: bold;
  /* margin-bottom: 5px; */
  color: #333;
}

.comment-text {
  font-size: 14px;
  color: #666;
  /* margin-bottom: 10px; */
}

.comment-date {
  font-size: 12px;
  color: #999;
}

.comment-form {
  display: block;
  flex: 1;
  margin-left: 25px;
}

.comment-input {
  width: 100%;
  padding: 10px;
  margin-bottom: 10px;
  border: 1px solid #ddd;
  border-radius: 8px;
}

/* 样式美化后提交按钮 */
.comment-submit {
  background-color: #409eff;
  color: white;
  border: none;
  height: 50px;
  padding: 10px 15px;
  border-radius: 5px;
  cursor: pointer;
  margin-left: 40px;
}

/* 悬停在提交按钮上时 */
.comment-submit:hover {
  background-color: #66b1ff;
}

/* 提交按钮活跃状态时 */
.comment-submit:active {
  background-color: #3a8ee6;
}

.comment-list-container {
  padding-left: 10px;
  max-height: calc(100vh - 430px) !important;
  overflow-y: auto;
  /* 添加垂直滚动条 */
  // margin-bottom: 20px;
  -ms-overflow-style: none;
  scrollbar-width: none;
}

/* 隐藏滚动条 */
.comment-list-container::-webkit-scrollbar {
  display: none;
  /* Chrome, Safari 和 Opera */
}

.comment-form-container {
  display: flex;
  position: absolute;
  bottom: 0;
  /* 将底部边缘与父容器底部对齐 */
  left: 0;
  /* 将左边边缘与父容器左边对齐 */
  max-height: calc(100vh - 480px);
  width: 100%;
  /* padding: 10px; */
  box-shadow: 0 -2px 5px rgba(0, 0, 0, 0.1);
  /* 阴影效果 */
}

.el-tag {
  margin-right: 8px;
  margin-bottom: 8px;
}


.info-section {
  /* 使info-section的高度自动填满，让评论输入框出现在底部 */
  height: 100%;
  display: flex;
  flex-direction: column;

  .info-top {
    padding-left: 10px;
  }
}

.detail-comments {
  margin-right: 20px;
  flex-grow: 1;
  overflow-y: auto;
  position: relative;
}

.image-section {
  position: relative;
  width: 75%;
  background-size: cover;
  background-position: center;
  overflow: hidden;
}



.button-container {
  position: absolute;
  bottom: 20px;
  left: 20px;
  z-index: 1;
}

.el-button {
  background-color: rgba(221, 221, 221, 0.5);
  /* 使用浅灰色并设置50%透明度 */
  color: #fff;
  border: none;
  /* 去掉边框 */
  box-shadow: none;
  /* 如果需要移除按钮的阴影效果，可以加上这一行 */
}

.el-button:hover {
  background-color: rgba(221, 221, 221, 0.7);
  /* 鼠标悬停时，背景颜色加深至70%透明度 */
}

/* 如果需要统一调整所有按钮（包括文本按钮）的颜色和透明度，可以使用以下选择器 */
.el-button:not(.is-disabled):not([plain]) {
  background-color: rgba(221, 221, 221, 0.5);
  color: #fff;
  border: none;
  box-shadow: none;
}

.el-button:not(.is-disabled):not([plain]):hover {
  background-color: rgba(221, 221, 221, 0.7);
}

.full-image-container {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.8);
  display: flex;
  justify-content: center;
  align-items: center;
  z-index: 1000;
}

.full-image {
  max-width: 100%;
  max-height: 100%;
  object-fit: contain;
  cursor: pointer;
}

/* 添加淡入淡出动画 */
.fade-enter-active,
.fade-leave-active {
  transition: opacity .5s;
}

.fade-enter,
.fade-leave-to {
  opacity: 0;
}
</style>