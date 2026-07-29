import pytest
import subprocess
import os
import tempfile
import re

@pytest.fixture
def temp_log_file():
    fd, path = tempfile.mkstemp(suffix=".log") #создание временного файла для логов
    os.close(fd)
    yield path
    if os.path.exists(path):
        os.remove(path)


@pytest.fixture
def run_app(app_path):
    def _run(args, inputs):
        cmd = [str(app_path)] + args
        process = subprocess.Popen(
            cmd,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        stdout, stderr = process.communicate(inputs)
        return stdout, stderr, process.returncode

    return _run



class TestLogger:
    def test_app_exists(self, app_path):
        assert app_path.exists(), f"Приложение не найдено: {app_path}"


    def test_default_level_info(self, temp_log_file, run_app):
        inputs = "Hello World\nexit\n"
        _, _, code = run_app([temp_log_file, "INFO"], inputs)

        assert code == 0
        with open(temp_log_file, 'r') as f:
            content = f.read()
            assert "INFO Hello World" in content


    def test_filter_debug_with_info(self, temp_log_file, run_app):
        inputs = "Test DEBUG\nexit\n"
        _, _, code = run_app([temp_log_file, "INFO"], inputs)

        assert code == 0
        with open(temp_log_file, 'r') as f:
            content = f.read()
            assert "DEBUG" not in content


    def test_filter_error_with_info(self, temp_log_file, run_app):
        inputs = "Test ERROR\nexit\n"
        _, _, code = run_app([temp_log_file, "INFO"], inputs)

        assert code == 0
        with open(temp_log_file, 'r') as f:
            content = f.read()
            assert "ERROR Test" in content


    def test_debug_level_all_messages(self, temp_log_file, run_app):
        inputs = "Debug1 DEBUG\nInfo1 INFO\nError1 ERROR\nexit\n"
        _, _, code = run_app([temp_log_file, "DEBUG"], inputs)

        assert code == 0
        with open(temp_log_file, 'r') as f:
            content = f.read()
            assert "DEBUG Debug1" in content
            assert "INFO Info1" in content
            assert "ERROR Error1" in content


    def test_error_level_only_errors(self, temp_log_file, run_app):
        inputs = "Debug DEBUG\nInfo INFO\nError ERROR\nexit\n"
        _, _, code = run_app([temp_log_file, "ERROR"], inputs)

        assert code == 0
        with open(temp_log_file, 'r') as f:
            content = f.read()
            assert "DEBUG" not in content
            assert "INFO" not in content
            assert "ERROR Error" in content


    def test_message_without_level_uses_default(self, temp_log_file, run_app):
        inputs = "No level\nexit\n"
        _, _, code = run_app([temp_log_file, "INFO"], inputs)

        assert code == 0
        with open(temp_log_file, 'r') as f:
            content = f.read()
            assert "INFO No level" in content


    def test_multiple_messages(self, temp_log_file, run_app):
        inputs = "First\nSecond ERROR\nThird DEBUG\nFourth\nexit\n"
        _, _, code = run_app([temp_log_file, "INFO"], inputs)

        assert code == 0
        with open(temp_log_file, 'r') as f:
            content = f.read()
            lines = content.strip().split('\n')

            assert len(lines) == 3
            assert "INFO First" in lines[0]
            assert "ERROR Second" in lines[1]
            assert "INFO Fourth" in lines[2]
            assert not any("Third" in line for line in lines)


    def test_log_format(self, temp_log_file, run_app):
        inputs = "Format test\nexit\n"
        _, _, code = run_app([temp_log_file, "INFO"], inputs)

        assert code == 0
        with open(temp_log_file, 'r') as f:
            content = f.read()
            # Формат: ГГГГ-ММ-ДД ЧЧ:ММ:СС УРОВЕНЬ Сообщение
            pattern = r'\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2} INFO Format test'
            assert re.search(pattern, content), "Неверный формат лога"


    def test_exit_command(self, temp_log_file, run_app):
        inputs = "Test\nexit\n"
        _, _, code = run_app([temp_log_file, "INFO"], inputs)
        assert code == 0


    def test_log_file_created(self, temp_log_file, run_app):
        inputs = "Test\nexit\n"
        _, _, code = run_app([temp_log_file, "INFO"], inputs)

        assert code == 0
        assert os.path.exists(temp_log_file)
        assert os.path.getsize(temp_log_file) > 0


    def test_empty_input(self, temp_log_file, run_app):
        _, _, code = run_app([temp_log_file, "INFO"], "")
        assert code == 0