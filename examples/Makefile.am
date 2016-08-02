# E1.31 (sACN) library for C/C++
# Hugo Hromic - http://github.com/hhromic
#
# Copyright 2016 Hugo Hromic
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

AM_CPPFLAGS = -I $(top_srcdir)/src
AM_CFLAGS = -Wall -O3 -std=gnu11 -pedantic

noinst_PROGRAMS = test_client test_server

test_client_SOURCES = test_client.c
test_client_LDADD = $(top_srcdir)/src/.libs/libe131.la
test_client_LDFLAGS = -static

test_server_SOURCES = test_server.c
test_server_LDADD = $(top_srcdir)/src/.libs/libe131.la
test_server_LDFLAGS = -static
