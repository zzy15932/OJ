function toggleForm() {
    // 获取登录表单和注册表单元素
    const loginForm = document.getElementById('loginForm');
    const registerForm = document.getElementById('registerForm');

    // 切换表单的显示状态
    if (loginForm.style.display === 'none') {
        loginForm.style.display = 'block';
        registerForm.style.display = 'none';
    } else {
        loginForm.style.display = 'none';
        registerForm.style.display = 'block';
    }
}

function validateLogin() {
    // 获取用户输入的邮箱和密码
    const email = document.getElementById('loginEmail').value;
    const password = document.getElementById('loginPassword').value;

    // 简单的前端验证：检查是否为空
    if (email === '' || password === '') {
        alert('请填写所有字段');
        return false;
    }

    // TODO: 添加更完善的验证逻辑和与后端的交互

    // 模拟登录成功跳转
    // alert('登录成功！');
    window.location.href = 'index.html'; 
    return false; 
}

function validateRegister() {
    // 获取用户输入的用户名、邮箱、密码和确认密码
    const username = document.getElementById('registerUsername').value;
    const email = document.getElementById('registerEmail').value;
    const password = document.getElementById('registerPassword').value;
    const confirmPassword = document.getElementById('confirmPassword').value;

    // 简单的前端验证：检查是否为空
    if (username === '' || email === '' || password === '' || confirmPassword === '') {
        alert('请填写所有字段');
        return false;
    }

    // 简单的前端验证：检查密码是否一致
    if (password !== confirmPassword) {
        alert('密码不匹配');
        return false;
    }

    // TODO: 添加更完善的验证逻辑和与后端的交互

    // 模拟注册成功跳转
    // alert('注册成功！');
    window.location.href = 'index.html'; 
    return false; 
}
