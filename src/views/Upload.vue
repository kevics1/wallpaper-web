<template>
  <div class="upload-wallpaper-container">
    <el-row :gutter="20">
      <!-- 左侧上传区域 -->
      <el-col :span="5">
        <div class="upload-container">
          <el-upload class="upload" action="#" :auto-upload="false" list-type="picture-card" :on-change="handleChange"
            :file-list="fileList" :on-exceed="handleExceed" limit="1">
            <i class="el-icon-plus"></i>
          </el-upload>
        </div>
      </el-col>

      <!-- 右侧表单区域 -->
      <el-col :span="8">
        <el-card class="box-card">
          <el-form ref="uploadForm" label-width="80px">
            <el-form-item label="标题">
              <el-input v-model="form.title"></el-input>
            </el-form-item>
            <el-form-item label="分类">
              <el-select v-model="form.categories" multiple filterable placeholder="请选择分类">
                <el-option v-for="item in categories" :key="item.value" :label="item.label" :value="item">
                </el-option>
              </el-select>
            </el-form-item>
            <!-- 显示所选分类 -->
            <el-form-item v-if="form.categories.length">
              <div class="selected-categories">
                <el-tag v-for="category in form.categories" :key="category" closable
                  @close="handleCategoryClose(category)">
                  {{ category.label }}
                </el-tag>
              </div>
            </el-form-item>
            <!-- <el-form-item label="分辨率">
              <el-input v-model="form.resolution" placeholder="例如：1920x1080"></el-input>
            </el-form-item> -->
            <el-form-item>
              <el-button type="primary" :loading="loading" @click="submitForm">上传壁纸</el-button>
            </el-form-item>
          </el-form>
        </el-card>
      </el-col>
    </el-row>

    <!-- 添加上传提示 -->
    <div class="upload-rules">
      <h3>稍等片刻！</h3>
      <p>我们很高兴您热衷于为czh-wallpaper做出贡献，但我们希望您能先花点时间熟悉我们的规则！请确保阅读以下所有规则，了解我们接受哪些类型的壁纸！</p>
      <p>请记住：这个网站用来分享壁纸的。我们不是某个随意的图片库！</p>
      <p>在上传前，问问自己：“这是壁纸吗？有人能将其用作壁纸吗？”如果答案是否定的，请不要上传！</p>
      <ul>
        <li>尊重版权。尽可能归属作者。不要上传内容。不要移除艺术家的签名/水印。</li>
        <li>
          严禁性内容和其他过分性感的图片。
          <a href="#" class="details-link" @click="toggleDetails('rule2')">详情...</a>
          <div v-if="details.rule2" class="details-container">
            性内容包括：
            <ul>
              <li>自慰、口交、性玩具、性交、体液等。</li>
              <li>生殖器的特写，专注于生殖器。</li>
              <li>过分性感的姿势：双腿张开、裸体弯腰、手放在生殖器上、展示生殖器、生殖器的拉伸/张开等。</li>
              <li>儿童色情（包括萝莉控/正太控）。</li>
            </ul>
            这不包括：
            <ul>
              <li>全裸。</li>
              <li>暴露生殖器（非过分性感的姿势）。</li>
            </ul>
          </div>
        </li>
        <li>不要将图片拉伸到更高分辨率——尽可能上传原始版本。</li>
        <li>
          禁止低质量编辑（旋转、镜像或画中画图片，拼凑在一起的拼贴画）。
          <a href="#" class="details-link" @click.prevent="toggleDetails('rule4')">详情...</a>
          <div v-if="details.rule4" class="details-container">
            <!-- 添加的详细内容 -->
            <p>低质量可能意味着：</p>
            <ul>
              <li>放大图片：将较小的图片尺寸增大。在全分辨率查看时，这总会导致质量损失。</li>
              <li>高水平的失真：由ISO值设置过高引起的噪点，高水平的JPEG/压缩失真，颜色带状。</li>
              <li>糟糕的编辑：边缘参差不齐且非风格化的渲染，'糟糕的Photoshop'。</li>
              <li>一些相机原始拍摄：对于'原始尺寸'规则的罕见例外；一些在标准壁纸尺寸下可以接受的拍摄，在未缩放时看起来可能很差。如果上传照片，请考虑它们在全分辨率下的外观。</li>
              <li>低质量扫描，文字照片等。</li>
              <li>横向图片的最小分辨率为1280x800，纵向图片为800x1280。</li>
            </ul>
            <p>这不包括：</p>
            <ul>
              <li>为艺术价值添加/使用的噪点。</li>
              <li>“风格化低质量”，如vaporwave或故障艺术。</li>
            </ul>
          </div>
        </li>
        <li>
          定义“套图”为在同一地点或非常相似的地点拍摄的同一主题的一系列照片。
          <a href="#" class="details-link" @click.prevent="toggleDetails('rule5')">详情...</a>
          <div v-if="details.rule5" class="details-container">
            <!-- 添加的详细内容 -->
            <ul>
              <li>此规则不仅限于单一用户的上传。换句话说，如果套图的部分已经上传，该套图的任何额外上传都将被移除。</li>
              <li>使用相同或相似提示生成的AI艺术作品被视为同一套图的一部分。</li>
            </ul>
          </div>
        </li>
        <li>禁止上传私人照片（家人、朋友、您的假期等）或业余色情内容。</li>
        <li>禁止上传宣传材料（即带有大型水印的），红地毯/新闻发布会照片或低分辨率样本。</li>
        <li>仅允许高质量的电影、电视剧、动漫和视频游戏截图。截图不得包含叠加层/HUDs/标志。不允许桌面截图。</li>
        <li>禁止血腥或其他令人不安的内容。</li>
        <li>上传多样化的图片集时，请分别标记和标识，以便内容准确分类。</li>
        <li>禁止展示或宣传仇恨或宣传材料，除非在小说、艺术、恶搞等中无害使用。</li>
      </ul>
      <p>我们保留随时更改这些规则和/或从网站上移除任何内容的权利。</p>
      <p>注意：这些规则可能随时更新。</p>
      <!-- <p>请花一分钟时间阅读我们的其余规定！</p> -->
    </div>
  </div>
</template>

<script>
import { saveWallpaper } from '@/api/wallpaper';

export default {
  data() {
    return {
      // 使用一个对象来管理所有可展开/折叠内容的显示状态
      details: {
        rule2: false,
        rule4: false,
        rule5: false,
        // 更多规则的状态...
      },
      loading: false, // 控制加载状态的数据属性
      form: {
        title: '',
        categories: [],
        file: {},
      },
      fileList: [], // 存储已上传的文件列表
      categories: [
        { value: '1', label: '风景' },
        { value: '2', label: '艺术' },
        { value: '3', label: '科技' },
        // ...
      ],
    };
  },
  methods: {
    beforeUpload(file) {
      // 创建一个FormData对象
      let formData = new FormData();
      // 添加文件对象
      formData.append('file', file);
      // 添加图片名称
      formData.append('name', file.name);
      // 可以继续添加其他需要上传的数据
      // ...

      // 修改请求的headers和data
      let config = {
        headers: {
          'Content-Type': 'multipart/form-data'
        },
        data: formData
      };

      // 返回false以阻止默认上传行为
      return false;
    },
    handlePreview(file) {
      // 预览图片的处理函数
    },
    handleRemove(file, fileList) {
      // 删除图片的处理函数
    },
    handleCategoryClose(removedCategory) {
      this.form.categories = this.form.categories.filter(category => category !== removedCategory);
    },
    submitForm() {
      console.log(this.form);
      this.saveWallpaper();
    },
    async saveWallpaper() {
      if (this.fileList.length === 0) {
        this.loading = false;
        this.$errorMessage('请选择壁纸');
        return;
      }
      this.loading = true; // 开始上传时设置loading为true

      await saveWallpaper({ title: this.form.title, categoryIds: this.form.categories, file: this.form.file }).then(res => {
        // 请求结束后取消加载状态
        this.loading = false;
        if (res.code == 200) {
          this.$successMessage('上传成功');
          this.form = {
            title: '',
            categories: [],
            file: {},
          };
          this.fileList = [];
        } else {
          this.$errorMessage(res.msg);
        }
      }).catch(error => {
        // 如果请求失败也要确保取消加载状态
        this.loading = false;
        this.$message.error(error.message);
      });
    },
    // 切换指定规则详细内容的显示状态
    toggleDetails(rule) {
      console.log(rule)
      console.log(this.details[rule])
      this.details[rule] = !this.details[rule];
    },
    handleChange(file, fileList) {
      this.fileList = fileList;
      fileList.forEach(fileItem => {
        this.form.file = fileItem.raw;
      });
    },
    handleExceed(files, fileList) {
      // 文件超出限制时的钩子
      this.$message.warning(`当前限制选择 1 个文件，本次选择了 ${files.length} 个文件，共选择了 ${files.length + fileList.length} 个文件`);
    }
  }
};
</script>

<style scoped>
.upload-wallpaper-container {
  margin: 30px;
}

.upload-container {
  /* border: 1px dashed #d9d9d9; */
  /* border-radius: 6px; */
  padding: 20px;
  text-align: center;
}

.upload::after {
  content: "";
  display: table;
  clear: both;
}

.box-card {
  border-radius: 10px;
}

.selected-categories {
  display: flex;
  flex-wrap: wrap;
  gap: 10px;
}

.upload-rules {
  margin-top: 20px;
  padding: 15px;
  border: 1px solid #eee;
  border-radius: 5px;
  background-color: #f9f9f9;
}

.upload-rules h3 {
  margin-top: 0;
}

.upload-rules ul {
  list-style-type: decimal;
  padding-left: 20px;
}

.upload-rules a {
  color: #007bff;
  cursor: pointer;
  text-decoration: underline;
}

.upload-rules a:hover {
  text-decoration: none;
}

.details-container {
    margin-top: 10px;
    padding: 10px;
    border: 1px solid #ddd;
    border-radius: 4px;
    background-color: #f9f9f9;
  }

  .details-container ul {
    list-style-type: none;
    padding: 0;
  }

  .details-container li {
    margin-bottom: 5px;
    line-height: 1.6;
    position: relative;
    padding-left: 25px; /* 留出空间用于图标或序号 */
  }

  .details-container li::before {
    content: '•'; /* 使用圆点作为默认图标 */
    position: absolute;
    left: 0;
    top: 0;
    color: #464646; /* 图标颜色 */
    font-size: 20px; /* 图标大小 */
    line-height: 1.2;
  }

  .details-link {
    color: #007bff;
    cursor: pointer;
    text-decoration: underline;
  }

  .details-link:hover {
    color: #0056b3;
  }
</style>