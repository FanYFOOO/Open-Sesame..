// start.js

Page({
    data: {

    },
    //跳转到开门记录页面
    navigate: function() {
        wx.navigateTo({
            url: '../door/door',
        })
    }
})