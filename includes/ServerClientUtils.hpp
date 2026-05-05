/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerClientUtils.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gafreire <gafreire@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 20:05:00 by gafreire          #+#    #+#             */
/*   Updated: 2026/05/05 20:05:00 by gafreire         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCLIENTUTILS_HPP
#define SERVERCLIENTUTILS_HPP

#include "bookstore.hpp"

namespace ServerClientUtils
{
    std::string toLower(const std::string& value);
    std::string trimSpaces(const std::string& value);
    bool getHeaderValue(const std::string& headers, const std::string& name, std::string& outValue);
    bool isChunkedBodyComplete(const std::string& buffer, size_t bodyStart);
}

#endif
