<template>
  <div class="message-board">
    <!-- 留言表单 -->
    <div class="sticky-note">
      <el-form ref="留言表单" label-position="top" :model="formData" :rules="rules">
        <el-form-item label="留言内容" prop="content">
          <el-input type="textarea" class="custom-textarea" v-model="formData.content"
            :autosize="{ minRows: 4, maxRows: 8 }" placeholder="请输入留言内容"></el-input>
        </el-form-item>
        <el-form-item>
          <el-button type="primary" @click="submitForm">提交留言</el-button>
        </el-form-item>
      </el-form>
    </div>

    <!-- 留言列表 -->
    <div class="comments">
      <transition-group name="fade-comment">
        <div class="comment-card" v-for="(comment, index) in comments" :key="index">
          <div class="comment-header">
            <span class="comment-author">{{ comment.userName == null ? '游客' : comment.userName }}</span>
            <span class="comment-time">{{ comment.postTime }}</span>
            <el-button size="mini" type="text" @click="replyTo(comment)">回复</el-button>
            <el-button size="mini" icon="el-icon-thumb" @click="likeComment(comment)"></el-button>
          </div>
          <p class="comment-content">{{ comment.content }}</p>
        </div>
      </transition-group>
      <!-- 加载更多按钮 -->
      <el-button v-if="hasMore" @click="loadMoreComments" type="text" plain>加载更多...</el-button>
    </div>

  </div>
</template>

<script>
import { getMessageBoards } from '@/api/messageBoard';
import { addMessageBoard } from '@/api/messageBoard';

export default {
  data() {
    return {
      formData: {
        content: '',
      },
      rules: {
        content: [{ required: true, message: '请填写留言内容', trigger: 'blur' }],
      },
      comments: [],
      hasMore: true,
      currentPage: 1,
      pageSize: 10,
    };
  },
  created() {
    this.loadMoreComments();
  },
  methods: {
    submitForm() {
      this.sendMessageBoard(this.formData.content);
    },
    async loadMoreComments() {
      await getMessageBoards(this.currentPage, this.pageSize).then((res) => {
        if (res.code != 200) {
          this.$errorMessage(res.msg);
        } else {
          this.comments = res.data.records;
          this.currentPage++;
          this.hasMore = res.data.total > this.comments.length;
        }
      });
    },
    async sendMessageBoard(text) {
      await addMessageBoard({ content: text }).then((res) => {
        if (res.code == 200) {
          this.$successMessage('留言成功');
          this.formData.content = '';
          this.comments.unshift(res.data);
        } else {
          this.$errorMessage(res.msg)
        }
      });
    },
    // 增加点赞和回复的功能逻辑
    likeComment(comment) {
      // 实现点赞逻辑，例如更新评论点赞数，调用API等
    },

    replyTo(comment) {
      // 打开回复框或者其他操作的逻辑
    },
  },
};
</script>

<style scoped lang="scss">
.message-board {
  box-shadow: 0 2px 4px rgba(0, 0, 0, 1);
  /* 添加阴影效果 */
  border-radius: 8px;
  /* 圆角边框 */
  padding: 20px;
  /* 内边距 */
  background-color: #e4e4e4;
  /* 背景颜色 */
  overflow: hidden;
  /* 隐藏溢出的内容 */

  width: auto;
  height: 100vh;
  /* 设置高度为视窗高度 */
  display: flex;
  flex-direction: column;
  overflow-y: auto;
  /* 添加滚动条，以防内容超出屏幕高度 */
  // background-color: #b8b8b8;
  padding: 25px;
}


.sticky-note {
  flex: 0 0 auto;
  /* 使其不参与拉伸 */
  padding: 20px;
  background-color: #fffa65;
  box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
  border-radius: 8px;
  margin-bottom: 30px;
  position: relative;
  transform: rotate(-2deg);
  width: 40%;
}

.comments {
  flex: 1 1 auto;
  /* 让其能自适应剩余空间 */
  overflow-y: auto;
  /* 添加滚动条 */
  margin-top: 20px;
}

.comment-card {
  /*width: 48%;  每行显示两个留言时的宽度 */

  width: 30%;
  /* 或者/* 每行显示三个留言时的宽度 */
  float: left;
  /* 使用float布局 */
  background-color: #fff;
  border-radius: 8px;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.4);
  padding: 15px;
  margin-bottom: 10px;
  transition: all 0.3s ease;
  margin: 10px;
}

.comment-header {
  display: flex;
  justify-content: space-between;
  margin-bottom: 10px;
}

.comment-author {
  font-weight: bold;
}

.comment-time {
  color: #999;
  font-size: 0.8em;
}

.comment-content {
  // box-shadow: 0 1px 3px rgba(0, 0, 0, 0.4); /* 添加阴影效果 */
  border-radius: 4px;
  /* 圆角边框 */
  padding: 10px;
  /* 内边距 */
  background-color: #ffffff;
  /* 背景颜色 */
  margin-bottom: 10px;
  /* 底部外边距，用于分隔卡片 */


  word-break: break-word; // 当单词过长无法放在一行时，将在单词内换行
  overflow-wrap: break-word; // 在长单词或URL地址内部进行换行
}

.list-enter-active,
.list-leave-active {
  transition: all 0.5s ease;
}

.list-enter,
.list-leave-to {
  opacity: 0;
  transform: translateY(30px);
}


.fade-comment-enter-active,
.fade-comment-leave-active {
  transition: all 0.5s ease;
}

.fade-comment-enter,
.fade-comment-leave-to {
  opacity: 0;
  transform: translateY(30px);
}

/* 回复和点赞按钮样式 */
.comment-header .el-button {
  margin-left: 10px;
}

/* 加载更多按钮样式 */
.load-more-btn {
  text-align: center;
  margin-top: 20px;
}



/* 针对Webkit浏览器 */
.message-board::-webkit-scrollbar {
  display: none;
  /* 隐藏滚动条 */
}

/* 针对Firefox */
.message-board {
  scrollbar-width: none;
  /* 隐藏滚动条 */
}

/* 针对IE和Edge */
.message-board {
  -ms-overflow-style: none;
  /* 隐藏滚动条 */
}
</style>