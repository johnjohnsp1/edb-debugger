/*
Copyright (C) 2006 - 2017 Evan Teran
                          evan.teran@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ExpressionDialog.h"
#include "Expression.h"
#include "ISymbolManager.h"	
#include "Symbol.h"
#include "edb.h"

#include <QCompleter>
#include <QPushButton>

ExpressionDialog::ExpressionDialog(const QString &title, const QString prompt) : QDialog(edb::v1::debugger_ui),
	layout_(this),
	label_text_("Replace me"),
	button_box_(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal)
{
	setWindowTitle(title);
	label_text_.setText(prompt);
	connect(&button_box_, SIGNAL(accepted()), this, SLOT(accept()));
	connect(&button_box_, SIGNAL(rejected()), this, SLOT(reject()));

	layout_.addWidget(&label_text_);
	layout_.addWidget(&expression_);
	layout_.addWidget(&label_error_);
	layout_.addWidget(&button_box_);

	palette_error_.setColor(QPalette::WindowText, Qt::red);
	label_error_.setPalette(palette_error_);

	button_box_.button(QDialogButtonBox::Ok)->setEnabled(false);

	setLayout(&layout_);

	connect(&expression_, SIGNAL(textChanged(const QString&)), this, SLOT(on_text_changed(const QString&)));
	expression_.selectAll();

	QList<std::shared_ptr<Symbol>> symbols = edb::v1::symbol_manager().symbols();
	QList<QString> allLabels;

	for(const std::shared_ptr<Symbol> &sym: symbols)
	{
		allLabels.append(sym->name_no_prefix);
	}
	allLabels.append(edb::v1::symbol_manager().labels().values());

	QCompleter *completer = new QCompleter(allLabels);
	expression_.setCompleter(completer);
	allLabels.clear();
}

void ExpressionDialog::on_text_changed(const QString& text) {
	QHash<edb::address_t, QString> labels = edb::v1::symbol_manager().labels();
	edb::address_t resAddr = labels.key(text);

	bool retval = false;

	if (resAddr)
	{
		last_address_ = resAddr;
		retval = true;
	}
	else
	{
		Expression<edb::address_t> expr(text, edb::v1::get_variable, edb::v1::get_value);
		ExpressionError err;

		bool ok;
		last_address_ = expr.evaluate_expression(&ok, &err);
		if(ok) {
			retval = true;
		} else {
			label_error_.setText(err.what());
			retval = false;
		}
	}

	button_box_.button(QDialogButtonBox::Ok)->setEnabled(retval);
	if (retval) {
		label_error_.clear();
	}
}

edb::address_t ExpressionDialog::getAddress() {
	return last_address_;
}
